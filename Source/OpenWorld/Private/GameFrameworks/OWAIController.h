// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OWAIController.generated.h"

class AEnemyCharacter;

UCLASS()
class AOWAIController : public AAIController
{
	GENERATED_BODY()

public:
	AOWAIController();

	// ===== AI ========== //

	void GoTo(const FVector& Location);
	void ActivateReaction();

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void ReferencesInitializer();

	// ===== References ========== //

	UPROPERTY()
	TWeakObjectPtr<AEnemyCharacter> EnemyCharacter;

	// ===== AI ========== //

	/** Generate random target location */
	FVector DetermineTargetLocation(const FVector& Target);

	// ===== Combat ========== //
	
	/** Attacking */
	UPROPERTY(EditAnywhere, Category=Combat)
	float HitRange = 300.f;

	FTimerHandle AttackDelayHandler;

	UPROPERTY(EditAnywhere, Category=Combat)
	float AttackDelayMin = 1.5f;

	UPROPERTY(EditAnywhere, Category=Combat)
	float AttackDelayMax = 2.f;

	void CheckRange();
	
    FORCEINLINE void PerformAttack();

    /** Reactions */
    FTimerHandle ReactionDelay;
    FORCEINLINE void FinishedReaction();

	// ===== Perceptions ========== //

	UFUNCTION()
	virtual void OnTargetSense(AActor* Actor, FAIStimulus Stimulus);
};
