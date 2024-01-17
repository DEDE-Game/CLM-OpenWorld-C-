// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/OWCharacter.h"
#include "EnemyCharacter.generated.h"

class AOWAIController;
class AWeapon;
class UNavigationInvokerComponent;

UCLASS()
class AEnemyCharacter : public AOWCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	friend class AOWAIController;

	// ===== Lifecycle ========== //

	virtual void PossessedBy(AController* NewController) override;

	// ===== Combat ========== //

	virtual void OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& HitImpact) override;

protected:
	// ===== References ========== //

	UPROPERTY()
	TWeakObjectPtr<AOWAIController> AIController;

	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNavigationInvokerComponent> NavInvoker;	

	// ===== Combat ========== //

	UPROPERTY(EditAnywhere, Category=Combat)
	TSubclassOf<AWeapon> GivenWeapon;

	virtual void SetLockOn(AOWCharacter* Target) override;

private:
	void DefaultInitializer();
};
