// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameCharacterAnimation.generated.h"

class AGameCharacter;
class UCharacterMovementComponent;

UCLASS()
class UGameCharacterAnimation : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// ===== Lifecycles ========== //

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	// ===== References ========== //

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<AGameCharacter> GameCharacter;

	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement;

	void InitializeReferences();

	// ===== Movements ========== //

	UPROPERTY(BlueprintReadOnly, Category=Movements, meta=(AllowPrivateAccess="true"))
	float SpeedForward;

	UPROPERTY(BlueprintReadOnly, Category=Movements, meta=(AllowPrivateAccess="true"))
	float SpeedSide;

	/** Updating movements value */
	void CheckMovements();

};
