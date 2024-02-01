// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DaytimeManager.generated.h"

class ADirectionalLight;
class APostProcessVolume;

UCLASS()
class OPENWORLD_API ADaytimeManager : public AActor
{
	GENERATED_BODY()

public:	
	ADaytimeManager();

	// ===== Lifecycles ========== //

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	
protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void ReferencesInitializer();

	// ===== Components ========== //

	/** Used as actual sun's rotation to avoid gimble lock */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SunScene;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MoonMesh;

	// ===== Lightings ========== //

	UPROPERTY(EditInstanceOnly, Category=Lightings)
	TWeakObjectPtr<ADirectionalLight> Sun;

	UPROPERTY(EditInstanceOnly, Category=Lightings)
	TWeakObjectPtr<ADirectionalLight> Moon;

	void UpdateTime();

	// ===== Attributes ========== //

	UPROPERTY(EditAnywhere, Category=Attributes, meta=(UIMin=0, UIMax=24))
	float Time = 0.f;

	UPROPERTY(EditAnywhere, Category=Attributes)
	float TimeRate = .1f;
};
