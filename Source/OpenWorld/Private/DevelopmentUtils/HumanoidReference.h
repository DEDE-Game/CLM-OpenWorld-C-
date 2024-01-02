// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HumanoidReference.generated.h"

/**
 * Placing humanoid references along the world 
 */
UCLASS()
class AHumanoidReference : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHumanoidReference();

	// ===== Generating ========== //

	UFUNCTION(CallInEditor, Category=Utility)
	void AddHumanoidReference();

	UFUNCTION(CallInEditor, Category=Utility)
	void ClearHumanoidRefenrence();

private:
	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> DummyReference;

	// ===== Attributes ========== //

	UPROPERTY(EditInstanceOnly, Category=Utility)
	FVector GridOrigin;

	UPROPERTY(EditInstanceOnly, Category=Utility)
	float TraceHeight = 10000.f;

	UPROPERTY(EditInstanceOnly, Category=Utility)
	FVector2D GridSize;

	UPROPERTY(EditInstanceOnly, Category=Utility)
	float Gap;
};
