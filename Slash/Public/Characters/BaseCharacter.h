// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

// Forward Declarations
class AWeapon;
class UAnimMontage;
class UAttributeComponent;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void Attack();

	

	void PlayHitReactMontage(FName SectionName);
	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	void DisableCapsule();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	virtual bool CanAttack();
	bool IsAlive();
	virtual void Die();

	UPROPERTY(VisibleAnywhere, Category = Weapon);
	AWeapon* EquippedWeapon;

	/**
	* Animation Montages
	*/
	UPROPERTY(EditDefaultsOnly, Category = Montages);
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages);
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages);
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> AttackMontageSections;
	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> DeathMontageSections;

	/**
	* Components
	*/
	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;


private:
	/**
	* Sounds/Particles
	*/
	UPROPERTY(EditAnywhere, Category = Sounds);
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualEffects);
	UParticleSystem* HitParticles;

public:	


};
