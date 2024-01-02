// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RainThunder.generated.h"

class APostProcessVolume;
class UMaterialParameterCollectionInstance;
class UNiagaraComponent;
class UNiagaraSystem;
class UTimelineComponent;

UCLASS()
class ARainThunder : public AActor
{
	GENERATED_BODY()
	
public:	
	ARainThunder();

	// ===== Lifecycles ========== //

	virtual void Tick(float DeltaTime) override;

	// ===== Interfaces ========== //

	void Strike();
	void ToggleThunder(bool bEnabled);
	void ToggleRain(bool bEnabled);

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void DefaultInitializer();

	// ===== Lightings ========== //

	UPROPERTY(EditInstanceOnly, Category=Lightings)
	TWeakObjectPtr<APostProcessVolume> PostProcessVolume;

	float DefaultExposureBias;

	// ===== GameFramework ========== //

	UPROPERTY()
	TWeakObjectPtr<APawn> PlayerPawn;

	// ===== Materials ========== //

	UPROPERTY(EditDefaultsOnly, Category=Materials)
	TSoftObjectPtr<UMaterialInterface> ParentThunderMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ThunderMaterial;

	UPROPERTY()
	TWeakObjectPtr<UMaterialParameterCollectionInstance> GlobalMatParamIns;

	void CreateMaterial();

	// ===== Rain ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> RainComponent;

	UPROPERTY(EditDefaultsOnly, Category=Rain)
	TSoftObjectPtr<UNiagaraSystem> RainVFX;

	UPROPERTY(EditDefaultsOnly, Category=Rain)
	TSoftObjectPtr<USoundBase> RainSFX;

	UPROPERTY()
	TObjectPtr<UAudioComponent> RainSound;

	bool bRaining = false;
	float RainLevel  = 0.f;
	float ChangeRate = .1f;

	void UpdateRain(float DeltaTime);

	// ===== Thunder ========== //

	/** This mesh will be applied with thunder material */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> PlaneMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTimelineComponent> ThunderTimeline;

	UPROPERTY(EditDefaultsOnly, Category=Thunder)
	TSoftObjectPtr<UCurveFloat> ThunderCurve;

	UPROPERTY(EditDefaultsOnly, Category=Thunder)
	TArray<TSoftObjectPtr<UTexture2D>> ThunderTextures;

	UPROPERTY(EditDefaultsOnly, Category=Thunder)
	TSoftObjectPtr<USoundBase> ThunderSFX;

	FTimerHandle ThunderTimerHandle;
	float ThunderTimerMin = 5.f;
	float ThunderTimerMax = 10.f;

	void SetupThunder();
	void UpdateThunderTimeline(float Value);

	/** Make the thunder always facing to player */
	void Relocate();

	/** Flashing effect just like real life thunder */
	void Flashing();

public:
	// ===== Modifiers ========== //

	void SetGlobalMatParamIns(UMaterialParameterCollectionInstance* InGlobalParam);
};
