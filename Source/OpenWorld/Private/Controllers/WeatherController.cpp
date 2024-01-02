// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/WeatherController.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Environments/RainThunder.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

UWeatherController::UWeatherController()
{
	// Actor
	PrimaryComponentTick.bCanEverTick 		   = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// PrimaryComponentTick.TickInterval = 1.f;

	DefaultInitializer();
}

// ==================== Initializer ==================== //

void UWeatherController::DefaultInitializer()
{
	ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> GlobalParam(
		TEXT("/Script/Engine.MaterialParameterCollection'/Game/Game/Materials/MP_Global.MP_Global'")
	);
	GlobalMatParam = GlobalParam.Object;

	ConstructorHelpers::FObjectFinder<UMaterialInterface> CloudMat(
		TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Game/Materials/Skies/MI_Cloud.MI_Cloud'")
	);
	CloudParentMaterial = CloudMat.Object;

	ConstructorHelpers::FClassFinder<ARainThunder> RainThunderBP(
		TEXT("/Game/Game/Blueprints/Environments/BP_RainThunder")
	);
	RainThunderClass = RainThunderBP.Class;
}

void UWeatherController::GetObjectReferences()
{
	// Global Params
	GlobalMatParamIns = GetWorld()->GetParameterCollectionInstance(GlobalMatParam.LoadSynchronous());
	PlayerPawn 		  = UGameplayStatics::GetPlayerPawn(this, 0);

	// Lightings
	SkyAtmosphere 	  = Cast<ASkyAtmosphere>	(UGameplayStatics::GetActorOfClass(this, ASkyAtmosphere	   ::StaticClass()));
	VolumetricCloud   = Cast<AVolumetricCloud>	(UGameplayStatics::GetActorOfClass(this, AVolumetricCloud  ::StaticClass()));

	PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(this, APostProcessVolume::StaticClass()));
	PostProcessVolume->Settings.bOverride_AutoExposureBias = true;

	// Set material for the volumetric cloud
	UVolumetricCloudComponent* CloudComponent = VolumetricCloud->GetComponentByClass<UVolumetricCloudComponent>();
	CloudMaterial = UMaterialInstanceDynamic::Create(CloudParentMaterial.LoadSynchronous(), this);
	CloudComponent->SetMaterial(CloudMaterial);
}

// ==================== Lifecycles ==================== //

void UWeatherController::BeginPlay()
{
	Super::BeginPlay();

	GetObjectReferences();
	PrepareEnvironments();

	// TESTING PURPOSE!!!
	// FTimerHandle Handle;
	// GetOwner()->GetWorldTimerManager().SetTimer(
	// 	Handle, [this]() { SetLightingValues(FLinearColor(.411458f, .411458f, .411458f), -1.5f, .5f); }, 3.f, false
	// );
	// FTimerHandle RainHandle;
	// GetOwner()->GetWorldTimerManager().SetTimer(
	// 	RainHandle, [this]() { RainThunder->ToggleRain(true); RainThunder->ToggleThunder(true); }, 6.f, false
	// );
	// FTimerHandle StopHandle;
	// GetOwner()->GetWorldTimerManager().SetTimer(StopHandle, [&] () {
	// 	RainThunder->ToggleThunder(false);
	// 	RainThunder->ToggleRain(false);
	// 	SetLightingValues(FLinearColor(.175287f, .409607f, 1.f), 1.f, 0.f); 
	// }, 66.f, false);
}

void UWeatherController::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ChangeLightingValues(DeltaTime);	

	// Watch the progress, if none simply disable the tick
	if (!bChangingLighting) SetComponentTickEnabled(false);
}

// ==================== Lightings ==================== //

void UWeatherController::ChangeLightingValues(float DeltaTime)
{
	if (!bChangingLighting) return;

	// Get smooth changing result by using interpolation
	CurrentAutoExposureBias 	= FMath::FInterpConstantTo(CurrentAutoExposureBias, TargetAutoExposureBias, DeltaTime, .09f);
	CurrentCloudDensity = FMath::FInterpConstantTo(CurrentCloudDensity, TargetCloudDensity, DeltaTime, .09f);
	CurrentAtmosphereColor = InterpLinearColor(DeltaTime, .09f);

	// Apply
	PostProcessVolume->Settings.AutoExposureBias = CurrentAutoExposureBias;
	CloudMaterial	 ->SetScalarParameterValue(TEXT("Density"), CurrentCloudDensity);
	SkyAtmosphere	 ->GetComponent()->SetRayleighScattering(CurrentAtmosphereColor);

	// Stop changing once its nearly equal
	if (FMath::IsNearlyEqual(CurrentAutoExposureBias, TargetAutoExposureBias, .0001f))
		bChangingLighting = false;
}

void UWeatherController::SetLightingValues(const FLinearColor& AtmosphereColor, float AutoExposureBias, float CloudDensity)
{
	TargetAtmosphereColor  = AtmosphereColor;
	TargetAutoExposureBias = AutoExposureBias;
	TargetCloudDensity 	   = CloudDensity;

	bChangingLighting = true;
	SetComponentTickEnabled(true);
}

FLinearColor UWeatherController::InterpLinearColor(float DeltaTime, float InterpSpeed)
{
	FLinearColor NewLinearColor;

	NewLinearColor.R = FMath::FInterpConstantTo(CurrentAtmosphereColor.R, TargetAtmosphereColor.R, DeltaTime, InterpSpeed);
	NewLinearColor.G = FMath::FInterpConstantTo(CurrentAtmosphereColor.G, TargetAtmosphereColor.G, DeltaTime, InterpSpeed);
	NewLinearColor.B = FMath::FInterpConstantTo(CurrentAtmosphereColor.B, TargetAtmosphereColor.B, DeltaTime, InterpSpeed);
	NewLinearColor.A = FMath::FInterpConstantTo(CurrentAtmosphereColor.A, TargetAtmosphereColor.A, DeltaTime, InterpSpeed);

	return NewLinearColor;
}

// ==================== Environments ==================== //

void UWeatherController::PrepareEnvironments()
{
	// Thunder
	RainThunder = GetWorld()->SpawnActor<ARainThunder>(RainThunderClass);
	RainThunder->SetGlobalMatParamIns(GlobalMatParamIns.Get());
}
