// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttributeComponent.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent(){
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UAttributeComponent::BeginPlay(){
	Super::BeginPlay();
	
}

void UAttributeComponent::RegenStamina(float DeltaTime) {
	Stamina = FMath::Clamp(Stamina + StaminaRegenRate * DeltaTime, 0.f, MaxStamina);
}

void UAttributeComponent::ReceiveDamage(float Damage) {
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
}

void UAttributeComponent::UseStamina(float StaminaCost) {
	Stamina = FMath::Clamp(Stamina - StaminaCost, 0.f, MaxStamina);
}

float UAttributeComponent::GetHealthPercent() {
	return Health / MaxHealth;
}

float UAttributeComponent::GetStaminaPercent() {
	return Stamina / MaxStamina;
}

bool UAttributeComponent::IsAlive() {
	return Health > 0.f;
}

void UAttributeComponent::AddSouls(int32 NumOfSouls) {
	Souls += NumOfSouls;
}

void UAttributeComponent::AddGold(int32 AmountOfGold) {
	Gold += AmountOfGold;
}

// Called every frame
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

