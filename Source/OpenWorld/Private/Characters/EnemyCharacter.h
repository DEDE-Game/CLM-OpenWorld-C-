// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/OWCharacter.h"
#include "EnemyCharacter.generated.h"

class AEnemyController;
class AMeleeWeapon;
class UHealthBar;
class UNavigationInvokerComponent;
class UWidgetComponent;

UCLASS()
class AEnemyCharacter : public AOWCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	friend class AEnemyController;

	// ===== Lifecycle ========== //

	virtual void PossessedBy(AController* NewController) override;
	virtual void Destroyed() override;

	// ===== Combat ========== //

	/*~ */
	virtual void OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& HitImpact, const float GivenDamage) override;

protected:
	// ===== References ========== //

	UPROPERTY()
	TWeakObjectPtr<AEnemyController> EnemyController;

	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNavigationInvokerComponent> NavInvoker;	

	// ===== Combat ========== //

	UPROPERTY(EditAnywhere, Category=Combat)
	TSubclassOf<AMeleeWeapon> GivenWeapon;

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
