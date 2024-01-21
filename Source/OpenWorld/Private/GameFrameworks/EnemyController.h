// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class AEnemyCharacter;

UCLASS()
class AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyController();

	// ===== Lifecycles ========== //

	virtual void Tick(float DeltaTime) override;

	// ===== AI ========== //

	void ActivateReaction();

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void ReferencesInitializer();

	// ===== References ========== //

	UPROPERTY()
	TWeakObjectPtr<AEnemyCharacter> EnemyCharacter;

	// ===== AI ========== //

	UFUNCTION()
	virtual void OnTargetSense(AActor* Actor, FAIStimulus Stimulus);

	// ===== Combat ========== //
	
	/** Attacking */
	UPROPERTY(EditAnywhere, Category=Combat)
	float HitRange = 300.f;

	void CheckRange();

    /** Reactions */
    FTimerHandle ReactionDelay;
    FORCEINLINE void FinishedReaction();

    // Get the decision randomly, but we can adjust the aggresivly
    /**
     * 0: Attack
     * 1: Strafing
	 * 2: Blocking
     */
	UPROPERTY(EditAnywhere, Category=Combat)
	TArray<int8> EngagingChances = { 0, 0, 1, 1, 1, 1, 2, 2 };

	/** Whether decide to strafe or attack */
	FTimerHandle EngageDelayHandle;

	UPROPERTY(EditAnywhere, Category=Combat)
	float EngageDelayMin = 1.f;

	UPROPERTY(EditAnywhere, Category=Combat)
	float EngageDelayMax = 6.f;

	void Engage();

	bool bStrafing = false;

	float StrafeDirectionX;
	float StrafeDirectionY;

	/** Strafing around player when on combat mode */
	void Strafing();
};
