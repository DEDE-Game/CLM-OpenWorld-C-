// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "OWAIController.generated.h"

class ACharacter;

UCLASS()
class AOWAIController : public AAIController
{
	GENERATED_BODY()

public:
	AOWAIController();

	// ===== AI ========== //

	void GoTo(const FVector& Location);

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void ReferencesInitializer();

	// ===== References ========== //

	UPROPERTY()
	TWeakObjectPtr<ACharacter> ControlledAI;

	// ===== AI ========== //

	/** Generate random target location */
	FVector DetermineTargetLocation(const FVector& Target);

	/** Get distance and direction between controlled AI and target location */
	void GetDistanceAndDirection(const FVector& Location, float& Distance, FVector& Direction);
};
