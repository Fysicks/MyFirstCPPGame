// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"

/* Components */
#include "HUD/HealthBarComponent.h"
#include "Components/AttributeComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setting up mesh to be hit by box trace of weapon, and stopping enemy from blocking camera
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	// Setting up health bar
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Setting up pawn sensing
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);
}

void AEnemy::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (IsDead()) { return; }
	if (EnemyState > EEnemyState::EES_Patrolling) {
		CheckCombatTarget();
	} else {
		CheckPatrolTarget();
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	
	if (IsInsideAttackRadius()) {
		EnemyState = EEnemyState::EES_Attacking;
	} else if (IsOutsideAttackRadius()){
		ChaseTarget();
	}

	return DamageAmount;
}

void AEnemy::Destroyed() {
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) {
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if(!IsDead()) ShowHealthBar();
	ClearPatrolTimer();
	ClearAttackTimer();
	StopAttackMontage();
	SetWeaponCollision(ECollisionEnabled::NoCollision);

	if (IsInsideAttackRadius() && !IsDead()) {
		StartAttackTimer();
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();


	if (PawnSensing) {
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
	InitializeEnemy();

	Tags.Add(FName("Enemy"));
}

/* 
* Base Character Overrides
*/
void AEnemy::Die_Implementation() {
	EnemyState = EEnemyState::EES_Dead;
	Super::Die_Implementation();
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	SetWeaponCollision(ECollisionEnabled::NoCollision);
	SpawnSoul();
}

void AEnemy::SpawnSoul() {
	UWorld* World = GetWorld();
	if (World && SoulClass && Attributes) {
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 25.f);
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, GetActorLocation(), GetActorRotation());
		if (SpawnedSoul) {
			SpawnedSoul->SetSouls(Attributes->GetSouls());
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Wtf it didn't work"))
		}
	}
}

void AEnemy::Attack() {
	Super::Attack();
	if (CombatTarget == nullptr) { return; }
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

bool AEnemy::CanAttack() {
	return IsInsideAttackRadius() && 
		!IsAttacking() && 
		!IsEngaged() &&
		!IsDead();;
}

void AEnemy::AttackEnd() {
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount) {
	Super::HandleDamage(DamageAmount);

	if (HealthBarWidget) {
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

int32 AEnemy::PlayAttackMontage() {
	return Super::PlayAttackMontage();
}

/*
* AI Behavior
*/
void AEnemy::InitializeEnemy() {
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	HideHealthBar();
	SpawnDefaultWeapon();
}

void AEnemy::CheckPatrolTarget() {
	if (InTargetRange(PatrolTarget, PatrolRadius)) {
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::CheckCombatTarget() {
	if (IsOutsideCombatRadius()) {
		ClearAttackTimer();
		LoseInterest();
		if (!IsEngaged()) { StartPatrolling(); }
	} else if (IsOutsideAttackRadius() && !IsChasing()) {
		ClearAttackTimer();
		if (!IsEngaged()) { ChaseTarget(); }
	} else if (CanAttack()) {
		StartAttackTimer();
	}

}

void AEnemy::PatrolTimerFinished() {
	MoveToTarget(PatrolTarget);
}

void AEnemy::ClearPatrolTimer() {
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::HideHealthBar() {
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar() {
	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(true);
	}
}

void AEnemy::LoseInterest() {
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling() {
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::ChaseTarget() {
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius() {
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius() {
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius() {
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing() {
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking() {
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead() {
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged() {
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::StartAttackTimer() {
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer() {
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius) {
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target) {
	if(EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget() {
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets) {
		if (Target != PatrolTarget) {
			ValidTargets.AddUnique(Target);
		}
	}
	const int32 NumPatrolTargets = ValidTargets.Num();

	if (NumPatrolTargets > 0) {
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}
	return nullptr;
}

void AEnemy::SpawnDefaultWeapon() {
	UWorld* World = GetWorld();
	if (GetWorld() && WeaponClass) {
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn) {
	const bool bShouldChaseTarget =
		EnemyState == EEnemyState::EES_Patrolling &&
		SeenPawn->ActorHasTag(FName("EngageableTarget")) &&
		!SeenPawn->ActorHasTag(FName("Dead"));

	if (bShouldChaseTarget) {
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}
