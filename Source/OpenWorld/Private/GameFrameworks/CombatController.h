// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CombatController.generated.h"

class ACombatCharacter;

UCLASS()
class ACombatController : public AAIController
{
	GENERATED_BODY()

public:
	ACombatController();

	// ***===== Lifecycles ==========*** //

	virtual void Tick(float DeltaTime) override;

	// ***===== AI ==========*** //

	void ActivateReaction();

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

protected:
	// ***===== Lifecycles ==========*** //

	virtual void BeginPlay() override;

private:
	void ReferencesInitializer();

	// ***===== References ==========*** //

	UPROPERTY()
	TWeakObjectPtr<ACombatCharacter> CombatCharacter;

	// ***===== AI ==========*** //

	// *** Sensing *** //

	bool bDisableSense = false;

	UFUNCTION()
	virtual void OnTargetSense(AActor* Actor, FAIStimulus Stimulus);

	// *** Decision *** //
    // Get the decision randomly, but we can adjust the aggresivly
    /**
     * 0: Attack
     * 1: Strafing
	 * 2: Blocking
     */
	UPROPERTY(EditAnywhere, Category=Combat)
	TArray<int8> DecisionChances = { 0, 0, 0, 0, 0, 1, 1, 1, 2 };

	/** Whether decide to strafe or attack */
	FTimerHandle DecisionDelayHandle;

	UPROPERTY(EditAnywhere, Category=Combat)
	float DecisionDelayMin = .7f;

	UPROPERTY(EditAnywhere, Category=Combat)
	float DecisionDelayMax = 3.5f;

	void Decide();

	FORCEINLINE void ReDecide()
	{
		GetWorldTimerManager().ClearTimer(DecisionDelayHandle);
		Decide();
	}

    // *** Reactions *** //
    FTimerHandle ReactionDelay;
    FORCEINLINE void FinishedReaction();

	// ***===== Patrolling ==========*** //

	FTimerHandle PatrollingDelayHandler;
	
	UPROPERTY(EditAnywhere, Category=Patrolling)
	float PatrollingDelayMin = 2.f;

	UPROPERTY(EditAnywhere, Category=Patrolling)
	float PatrollingDelayMax = 5.f;

	void StartPatrolling();

	// ***===== Combat ==========*** //
	
	// *** Attacking *** //
	UPROPERTY(EditAnywhere, Category=Combat)
	float HitRange = 300.f;

	void Attacking();

	// *** Strafing *** //
	bool bStrafing = false;

	float StrafeDirectionX;
	float StrafeDirectionY;

	/** Strafing around player when on combat mode */
	void Strafing();

	// *** Blocking *** //
	/** Disable blocking after certain time */
	FTimerHandle BlockingTimerHandle;
};
