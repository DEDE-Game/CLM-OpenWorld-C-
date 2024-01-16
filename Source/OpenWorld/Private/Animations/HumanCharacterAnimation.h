// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HumanCharacterAnimation.generated.h"

class AOWCharacter;
class UCharacterMovementComponent;

UCLASS()
class UHumanCharacterAnimation : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// ===== Lifecycles ========== //

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	// ===== References ========== //

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<AOWCharacter> OWCharacter;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement;

	void InitializeReferences();

	// ===== Movements ========== //

	UPROPERTY(BlueprintReadOnly, Category=Movements, meta=(AllowPrivateAccess="true"))
	float SpeedForward;

	UPROPERTY(BlueprintReadOnly, Category=Movements, meta=(AllowPrivateAccess="true"))
	float SpeedSide;

	UPROPERTY(BlueprintReadOnly, Category=Movements, meta=(AllowPrivateAccess="true"))
	bool bCrouched;

	/** Updating movements value */
	void UpdateMovements();

	// ===== Combat ========== //

	UPROPERTY(BlueprintReadOnly, Category=Combat, meta=(AllowPrivateAccess="true"))
	bool bEquipWeapon;

	/** Updating combats movement */
	void UpdateCombat();
};
