// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/OWCharacter.h"
#include "PlayerCharacter.generated.h"

class AOWPlayerController;
class AOWHUD;
struct FInputActionValue;
class UAnimMontage;
class UBoxComponent;
class UCameraComponent;
class UInputAction;
class UInventoryComponent;
class USpringArmComponent;

UCLASS()
class APlayerCharacter : public AOWCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	// ===== Lifecycles ========== //

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	// ===== Combat ========== //

	virtual void DeactivateAction() override;

	// ===== UI ========== //

	void ShowTip(const FString& Text);
	void HideTip();

protected:

	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

	// ===== Combat ========== //

	virtual void OnLostInterest() override;

private:
	void DefaultInitializer();
	void ReferencesInitializer();

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TakedownArea;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInventoryComponent> Inventory;

	// ===== References ========== //

	UPROPERTY(EditAnywhere, Category=Materials)
	TSoftObjectPtr<UMaterialParameterCollection> GlobalMatParam;

	UPROPERTY()
	TWeakObjectPtr<UMaterialParameterCollectionInstance> GlobalMatParamIns;

	UPROPERTY()
	TWeakObjectPtr<AOWPlayerController> OWPlayerController;

	UPROPERTY()
	TWeakObjectPtr<AOWHUD> OWHUD;

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

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> InteractAction;

	// ===== Locomotions ========== //

	FVector2D MovementInput;

	void Look(const FInputActionValue &InputValue);
	void Move(const FInputActionValue &InputValue);
	void DoJump();
	
	FORCEINLINE void Sprint(const FInputActionValue &InputValue);
	FORCEINLINE void Walk(const FInputActionValue &InputValue);
	FORCEINLINE void DoCrouch(const FInputActionValue& InputValue);

	virtual void Landed(const FHitResult &Hit) override;

	// ===== Combat ========== //

	FORCEINLINE void Block(const FInputActionValue& InputValue);
	FORCEINLINE void ChangeWeapon(const FInputActionValue& InputValue);

	void Dodge();

	virtual void Attack() override;

	/**== CHARGING ATTACK ==**/

	UPROPERTY(EditAnywhere, Category=Combat)
	float DamageMultiplierRate = .8f;

	float DamageMultiplier = 1.f;

	/** Once the time is passed, will use charge attack instead */
	FTimerHandle ChargeTimer;
	float ChargeAfter = .6f;

	bool bCharging = false;

	void ChargeAttack();
	void DoChargeAttack();

	// ===== Takedown ========== //

	UPROPERTY()
	TWeakObjectPtr<AOWCharacter> TargetTakedown;

	UFUNCTION()
	void OnEnterTakedown(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLeaveTakedown(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void PerformTakedown();
	
	// ===== Environments ========== //

	/**=== Foliages ===**/

	/** Delay foliage bending effect */
	FVector LastLocation1;
	FVector LastLocation2;

	/** Make nearby character's foliage bending */
	void AffectsFoliage();

	/**=== Game Objects ===**/

	UPROPERTY()
	TWeakObjectPtr<AMeleeWeapon> OverlappingWeapon;

	void Interact();

public:
	// ===== Modifiers ========== //

	FORCEINLINE void SetOverlappingWeapon(TWeakObjectPtr<AMeleeWeapon> Weapon)
	{
		OverlappingWeapon = Weapon;
	}
	FORCEINLINE void SetCarriedWeapon(TWeakObjectPtr<AMeleeWeapon> Weapon, bool bAllowedSwap = true)
	{
		CarriedWeapon = Weapon;
		bEquipWeapon  = CarriedWeapon.IsValid();
		bAllowSwapWeapon = bAllowedSwap;
	}
};
