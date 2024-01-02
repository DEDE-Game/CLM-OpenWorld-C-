// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UAnimMontage;
struct FInputActionValue;

UCLASS()
class AGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGameCharacter();

	// ===== Lifecycles ========== //

	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void DefaultInitializer();

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

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

	// ===== Locomotions ========== //

	UPROPERTY(EditAnywhere, Category = Locomotions)
	float SprintSpeed = 800.f;

	UPROPERTY(EditAnywhere, Category = Locomotions)
	float RunSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = Locomotions)
	float WalkSpeed = 200.f;

	void Look(const FInputActionValue &InputValue);
	void Move(const FInputActionValue &InputValue);
	void Sprint(const FInputActionValue &InputValue);
	void Walk(const FInputActionValue &InputValue);
	void DoJump();

	virtual void Landed(const FHitResult &Hit) override;

	// ===== Animations ========== //

	UPROPERTY(EditDefaultsOnly, Category = Animations)
	TMap<FName, TSoftObjectPtr<UAnimMontage>> Montages;

	// ===== Audio ========== //

	UPROPERTY(EditDefaultsOnly, Category = Audio)
	TMap<FName, TSoftObjectPtr<USoundBase>> FootstepSounds;

	UFUNCTION(BlueprintCallable)
	void PlayFootstepSound();
};
