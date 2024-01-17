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

	// ===== Lifecycles ========== //

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

	UPROPERTY(EditDefaultsOnly, Category=Combat)
	TSubclassOf<AWeapon> GivenWeapon;

	virtual void SetLockOn(AOWCharacter* Target) override;

	FTimerHandle ReactionDelay;
	
	FORCEINLINE void FinishedReaction();

	// ===== Perceptions ========== //

	UFUNCTION()
	virtual void OnTargetSense(AActor* Actor, FAIStimulus Stimulus);

private:
	void DefaultInitializer();
};
