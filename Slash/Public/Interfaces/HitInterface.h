// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHitInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SLASH_API IHitInterface
{
	GENERATED_BODY()

public:
	// Making pure function, will get overwritten in inherited classes
	// BlueprintNativeEvent will generate the virtual version of this function that 
	// You can override. It appends "_Implementation" to the name.
	// That's why we don't need to label this virtual even though we override it 
	// in other classes. Also don't need to provide a function implementation (the 
	// green squiggles) because of the BlueprintNativeEvent
	UFUNCTION(BlueprintNativeEvent)
	void GetHit(const FVector& ImpactPoint, AActor* Hitter);
};
