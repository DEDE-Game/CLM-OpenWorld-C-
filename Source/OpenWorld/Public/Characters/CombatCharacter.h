// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/OWCharacter.h"
#include "CombatCharacter.generated.h"

class ACombatController;
class AMeleeWeapon;
class APlayerCharacter;
class UHealthBar;
class UNavigationInvokerComponent;
class USoundBase;
class UWidgetComponent;

UCLASS()
class OPENWORLD_API ACombatCharacter : public AOWCharacter
{
	GENERATED_BODY()

public:
	ACombatCharacter();

	friend class ACombatController;

	// ===== Lifecycle ========== //

	virtual void PossessedBy(AController* NewController) override;
	virtual void Destroyed() override;

	// ===== Combat ========== //

	/*~ */
	virtual void OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& HitImpact, const float GivenDamage, bool bBlockable) override;

protected:
	// ===== References ========== //

	UPROPERTY()
	TWeakObjectPtr<ACombatController> EnemyController;

	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> AttackIndicator;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNavigationInvokerComponent> NavInvoker;

	// ===== Attributes ========== //

	virtual void Die() override;

	// ===== Combat ========== //

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<TSubclassOf<AMeleeWeapon>> GivenWeaponClasses;

	void RandomizeWeapon();

	virtual void AttackCombo() override;
	virtual void StartChargeAttack() override;
	virtual void EnableWeapon(bool bEnabled) override;
	virtual void AttachWeapon() override;
	virtual void SetLockOn(AOWCharacter *Target) override;

	// ===== UI ========== //

	/** The actual health bar */
	UPROPERTY()
	TWeakObjectPtr<UHealthBar> HealthBarWidget;

	// ===== Audio ========== //

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TSoftObjectPtr<USoundBase> SlowSFX;

private:
	void DefaultInitializer();
	void InitializeUI();
};
