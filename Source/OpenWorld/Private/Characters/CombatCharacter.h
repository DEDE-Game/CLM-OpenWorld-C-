// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/OWCharacter.h"
#include "CombatCharacter.generated.h"

class ACombatController;
class AMeleeWeapon;
class UHealthBar;
class UNavigationInvokerComponent;
class UWidgetComponent;

UCLASS()
class ACombatCharacter : public AOWCharacter
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
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNavigationInvokerComponent> NavInvoker;	

	// ===== Combat ========== //

	UPROPERTY(EditAnywhere, Category=Combat)
	TArray<TSubclassOf<AMeleeWeapon>> GivenWeaponClasses;

	void RandomizeWeapon();

	virtual void SwapWeapon() override;
	virtual void SetLockOn(AOWCharacter* Target) override;
	virtual void Die() override;

	// ===== UI ========== //

	/** The actual health bar */
	UPROPERTY()
	TWeakObjectPtr<UHealthBar> HealthBar;

private:
	void DefaultInitializer();
};
