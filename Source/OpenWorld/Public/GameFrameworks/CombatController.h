// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CombatController.generated.h"

class ACombatCharacter;

UCLASS()
class OPENWORLD_API ACombatController : public AAIController
{
	GENERATED_BODY()

public:
	ACombatController();

	// ***===== Lifecycles ==========*** //

	virtual void Tick(float DeltaTime) override;

	// ***===== AI ==========*** //

	FORCEINLINE void ActivateReaction();

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

	// *** Engaging *** //
    // Get the Engaging randomly, but we can adjust the aggresivly
    /**
     * 0: Attack
     * 1: Strafing
	 * 2: Blocking
	 * 3: Charge Attack
     */
	UPROPERTY(EditAnywhere, Category=AI)
	TArray<int8> EngageChances = { 0, 0, 0, 0, 1, 1, 2, 3 };

	/** Whether decide to strafe or attack */
	FTimerHandle EngageDelayHandle;

	UPROPERTY(EditAnywhere, Category=AI)
	float EngageDelayMin = .7f;

	UPROPERTY(EditAnywhere, Category=AI)
	float EngageDelayMax = 3.5f;

    void Blocking();

    FORCEINLINE void ReEngage()
    {
        GetWorldTimerManager().ClearTimer(EngageDelayHandle);
		Engage();
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
	float HitRange = 250.f;

	void Attacking();

	// *** Strafing *** //
	bool bStrafing = false;

	float StrafeDirectionX;
	float StrafeDirectionY;

    FORCEINLINE void StartStrafing();

	/** Strafing around player when on combat mode */
	void Strafing();

	// *** Blocking *** //
	/** Disable blocking after certain time */
	FTimerHandle BlockingTimerHandle;

	void Engage();
};
