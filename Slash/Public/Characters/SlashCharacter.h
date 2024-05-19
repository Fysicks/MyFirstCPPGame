// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h" // Needed for FInputActionValue
#include "CharacterTypes.h"
#include "SlashCharacter.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UGroomComponent; // Had to add "Niagara" to dependencies to get to work
class AItem;
class UAnimMontage;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	virtual void Jump() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	/**
	* Input Mapping Context and Input Actions
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DodgeAction;

	/**
	* Callbacks for Inputs
	*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EKeyPressed();
	virtual void Attack() override;
	void Dodge();

	/** 
	* Play Montage Functions and Helpers
	*/
	virtual void AttackEnd() override;
	virtual bool CanAttack() override;

	void PlayEquipMontage(FName SectionName);
	bool CanDisarm();
	bool CanArm();
	UFUNCTION(BlueprintCallable)
	void Disarm();
	UFUNCTION(BlueprintCallable)
	void Arm();
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

private:
	/**
	* States
	*/
	UPROPERTY(VisibleAnywhere)
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	/**
	* Components
	*/
	UPROPERTY(VisibleAnywhere);
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere);
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = Hair);
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair);
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly);
	AItem* OverlappingItem;

	/**
	* Animation Montages
	*/
	UPROPERTY(EditDefaultsOnly, Category = Montages);
	UAnimMontage* EquipMontage;

public: // Setters and getters
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
