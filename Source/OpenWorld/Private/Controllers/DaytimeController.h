// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DaytimeController.generated.h"

class ADirectionalLight;
class APostProcessVolume;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UDaytimeController : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDaytimeController();

	// ===== Lifecycles ========== //

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void ReferencesInitializer();

	// ===== Lightings ========== //

	UPROPERTY()
	TWeakObjectPtr<ADirectionalLight> Sun;

	UPROPERTY()
	TWeakObjectPtr<ADirectionalLight> Moon;

	void UpdateTime();

	// ===== Attributes ========== //

	UPROPERTY(EditAnywhere, Category=Attributes, meta=(UIMin=0, UIMax=24))
	float Time = 0.f;

	UPROPERTY(EditAnywhere, Category=Attributes)
	float TimeRate = .1f;
};
