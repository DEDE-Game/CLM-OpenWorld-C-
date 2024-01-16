// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/OWCharacter.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;

UCLASS()
class APlayerCharacter : public AOWCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	// ===== Lifecycles ========== //

	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

protected:

	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void DefaultInitializer();
	void ReferencesInitializer();

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	// ===== References ========== //

	UPROPERTY(EditAnywhere, Category=Materials)
	TSoftObjectPtr<UMaterialParameterCollection> GlobalMatParam;

	UPROPERTY()
	TWeakObjectPtr<UMaterialParameterCollectionInstance> GlobalMatParamIns;

	// ===== Input ========== //

	UPROPERTY(EditDefaultsOnly, Category = Input)
	TSoftObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	TSoftObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	TSoftObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	TSoftObjectPtr<UInputAction> WalkAction;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	TSoftObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> SwapWeaponAction;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> BlockAction;

	// ===== Locomotions ========== //

	void Look(const FInputActionValue &InputValue);
	void Move(const FInputActionValue &InputValue);
	void DoJump();
	
	FORCEINLINE void Sprint(const FInputActionValue &InputValue);
	FORCEINLINE void Walk(const FInputActionValue &InputValue);
	FORCEINLINE void DoCrouch(const FInputActionValue& InputValue);

	virtual void Landed(const FHitResult &Hit) override;

	// ===== Combat ========== //

	// !!THIS GIVEN WEAPON IS FOR TESTING ONLY!!
	UPROPERTY(EditAnywhere) TSubclassOf<AWeapon> GivenWeapon;

	FORCEINLINE void Block(const FInputActionValue& InputValue);
	FORCEINLINE void ChangeWeapon(const FInputActionValue& InputValue);

	/** Find nearest enemy then lock to him */
	void LockNearest();

	virtual void Attack() override;
	
	// ===== Environments ========== //

	/** Delay foliage bending effect */
	FVector LastLocation1;
	FVector LastLocation2;

	/** Make nearby character's foliage bending */
	void AffectsFoliage();
};
