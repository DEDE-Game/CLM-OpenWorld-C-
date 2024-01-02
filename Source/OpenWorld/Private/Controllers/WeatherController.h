// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeatherController.generated.h"

class APostProcessVolume;
class ARain;
class ASkyAtmosphere;
class ARainThunder;
class AVolumetricCloud;
class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;

UCLASS()
class UWeatherController : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UWeatherController();

	// ===== Lifecycles ========== //

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	// ===== Initializer ========== //

	void DefaultInitializer();
	void GetObjectReferences();

	// ===== References ========== //

	UPROPERTY()
	TWeakObjectPtr<APawn> PlayerPawn;

	UPROPERTY(VisibleInstanceOnly, Category=Materials)
	TSoftObjectPtr<UMaterialParameterCollection> GlobalMatParam;

	UPROPERTY()
	TWeakObjectPtr<UMaterialParameterCollectionInstance> GlobalMatParamIns;

	// ===== Lightings ========== //

	UPROPERTY(VisibleInstanceOnly, Category=Lightings)
	TWeakObjectPtr<ASkyAtmosphere> SkyAtmosphere;

	UPROPERTY(VisibleInstanceOnly, Category=Lightings)
	TWeakObjectPtr<APostProcessVolume> PostProcessVolume;

	UPROPERTY(VisibleInstanceOnly, Category=Lightings)
	TWeakObjectPtr<AVolumetricCloud> VolumetricCloud;

	UPROPERTY(VisibleInstanceOnly, Category=Materials)
	TSoftObjectPtr<UMaterialInterface> CloudParentMaterial;

	UPROPERTY(VisibleInstanceOnly, Category=Materials)
	TObjectPtr<UMaterialInstanceDynamic> CloudMaterial;

	/** Make changing lightings to be smooth */
	bool bChangingLighting = false;

	FLinearColor CurrentAtmosphereColor = FLinearColor(.175287f, .409607f, 1.f);
	float CurrentAutoExposureBias = 1.f;
	float CurrentCloudDensity = 0.f;

	FLinearColor TargetAtmosphereColor;
	float TargetAutoExposureBias;
	float TargetCloudDensity;

	/** Make changing lightings to be smooth */
	void ChangeLightingValues(float DeltaTime);

	/** Call this to change the lighting values */
	void SetLightingValues(const FLinearColor& AtmosphereColor, float AutoExposureBias, float CloudDensity);

	/** Function helper to interpolate linear color constantly */
	FLinearColor InterpLinearColor(float DeltaTime, float InterpSpeed);

	// ===== Environments ========== //

	/** RainThunder */
	TSubclassOf<ARainThunder> RainThunderClass;
	TObjectPtr<ARainThunder> RainThunder;

	void PrepareEnvironments();
};
