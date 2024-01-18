// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterface.generated.h"

class AOWCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHitInterface : public UInterface
{
	GENERATED_BODY()
};

class IHitInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// ===== Combat ========== //

	/** Check is the character in 1 team with other */
	FORCEINLINE virtual const bool IsEnemy(AOWCharacter* Other) const = 0;

	/** Check is the character on blocking state */
	FORCEINLINE virtual const bool IsBlocking() const = 0;
	
	virtual void OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& HitImpact) = 0;
};
