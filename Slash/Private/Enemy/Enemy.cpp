// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"

#include "HUD/HealthBarComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/KismetSystemLibrary.h"

#include "Items/Weapons/Weapon.h"
#include "Slash/DebugMacros.h"


// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setting up mesh to be hit by box trace of weapon, and stopping enemy from blocking camera
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

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

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (HealthBarWidget) {
		HealthBarWidget->SetVisibility(false);
	}

	EnemyController = Cast<AAIController>(GetController());

	MoveToTarget(PatrolTarget);

	if (PawnSensing) {
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	UWorld* World = GetWorld();
	if (World && WeaponClass) {
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::Die() {
	EnemyState = EEnemyState::EES_Dead;
	PlayDeathMontage();
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
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
	MoveRequest.SetAcceptanceRadius(50.f);
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

void AEnemy::Attack() {
	Super::Attack();
	PlayAttackMontage();
}

bool AEnemy::CanAttack() {
	return IsInsideAttackRadius() && 
		!IsAttacking() && 
		!IsDead();;
}

void AEnemy::HandleDamage(float DamageAmount) {
	Super::HandleDamage(DamageAmount);

	if (HealthBarWidget) {
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

int32 AEnemy::PlayAttackMontage() {
	return int32();
}

int32 AEnemy::PlayDeathMontage() {
	const int32 Selection = Super::PlayDeathMontage();
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_Max) {
		DeathPose = Pose;
	}
	return Selection;
}

void AEnemy::PawnSeen(APawn* SeenPawn) {
	const bool bShouldChaseTarget =
		EnemyState == EEnemyState::EES_Patroling &&
		SeenPawn->ActorHasTag(FName("SlashCharacter"));

	if (bShouldChaseTarget) {
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

void AEnemy::PatrolTimerFinished() {
	MoveToTarget(PatrolTarget);
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
	EnemyState = EEnemyState::EES_Patroling;
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

void AEnemy::ClearPatrolTimer() {
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::StartAttackTimer() {
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer() {
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) { return; }
	if (EnemyState > EEnemyState::EES_Patroling) {
		CheckCombatTarget();
	} else {
		CheckPatrolTarget();
	}
}

void AEnemy::CheckPatrolTarget() {
	if (InTargetRange(PatrolTarget, PatrolRadius)) {
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
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

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint) {
	ShowHealthBar();
	if (IsAlive()) { 
		DirectionalHitReact(ImpactPoint);
	} else { Die();}

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget();
	return DamageAmount;
}

void AEnemy::Destroyed() {
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}
}
