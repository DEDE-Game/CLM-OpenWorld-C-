// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/WeatherController.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Environments/RainThunder.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

AWeatherController::AWeatherController()
{
	// Actor
	PrimaryActorTick.bCanEverTick 		   = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	// PrimaryComponentTick.TickInterval = 1.f;

	DefaultInitializer();
}

void AWeatherController::DefaultInitializer()
{
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> GlobalParamAsset(
		TEXT("/Script/Engine.MaterialParameterCollection'/Game/Game/Materials/MP_Global.MP_Global'")
	);
	GlobalMatParam = GlobalParamAsset.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CloudMatAsset(
		TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Game/Materials/Skies/MI_Cloud.MI_Cloud'")
	);
	CloudParentMaterial = CloudMatAsset.Object;

	static ConstructorHelpers::FClassFinder<ARainThunder> RainThunderAsset(
		TEXT("/Game/Game/Blueprints/Environments/BP_RainThunder")
	);
	RainThunderClass = RainThunderAsset.Class;
}

void AWeatherController::ReferencesInitializer()
{
	// Global Params
	GlobalMatParamIns = GetWorld()->GetParameterCollectionInstance(GlobalMatParam.LoadSynchronous());
	PlayerPawn 		  = UGameplayStatics::GetPlayerPawn(this, 0);

	// Lightings
	SkyAtmosphere 	  = Cast<ASkyAtmosphere>  (UGameplayStatics::GetActorOfClass(this, ASkyAtmosphere  ::StaticClass()));
	VolumetricCloud   = Cast<AVolumetricCloud>(UGameplayStatics::GetActorOfClass(this, AVolumetricCloud::StaticClass()));

	// Set material for the volumetric cloud
	UVolumetricCloudComponent* CloudComponent = VolumetricCloud->GetComponentByClass<UVolumetricCloudComponent>();
	CloudMaterial = UMaterialInstanceDynamic::Create(CloudParentMaterial.LoadSynchronous(), this);
	CloudComponent->SetMaterial(CloudMaterial);
}

// ==================== Lifecycles ==================== //

void AWeatherController::BeginPlay()
{
	Super::BeginPlay();

	ReferencesInitializer();
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

void AWeatherController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ChangeLightingValues(DeltaTime);	

	// Watch the progress, if none simply disable the tick
	if (!bChangingLighting) SetActorTickEnabled(false);
}

// ==================== Lightings ==================== //

void AWeatherController::ChangeLightingValues(float DeltaTime)
{
	if (!bChangingLighting) return;

	// Get smooth changing result by using interpolation
	CurrentCloudDensity = FMath::FInterpConstantTo(CurrentCloudDensity, TargetCloudDensity, DeltaTime, .09f);
	CurrentAtmosphereColor = InterpLinearColor(DeltaTime, .09f);

	// Apply
	CloudMaterial	 ->SetScalarParameterValue(TEXT("Density"), CurrentCloudDensity);
	SkyAtmosphere	 ->GetComponent()->SetRayleighScattering(CurrentAtmosphereColor);

	// Stop changing once its nearly equal
	if (FMath::IsNearlyEqual(CurrentCloudDensity, TargetCloudDensity, .0001f))
		bChangingLighting = false;
}

void AWeatherController::SetLightingValues(const FLinearColor& AtmosphereColor, float AutoExposureBias, float CloudDensity)
{
	TargetAtmosphereColor  = AtmosphereColor;
	TargetCloudDensity 	   = CloudDensity;

	bChangingLighting = true;
	SetActorTickEnabled(true);
}

FLinearColor AWeatherController::InterpLinearColor(float DeltaTime, float InterpSpeed)
{
	FLinearColor NewLinearColor;

	NewLinearColor.R = FMath::FInterpConstantTo(CurrentAtmosphereColor.R, TargetAtmosphereColor.R, DeltaTime, InterpSpeed);
	NewLinearColor.G = FMath::FInterpConstantTo(CurrentAtmosphereColor.G, TargetAtmosphereColor.G, DeltaTime, InterpSpeed);
	NewLinearColor.B = FMath::FInterpConstantTo(CurrentAtmosphereColor.B, TargetAtmosphereColor.B, DeltaTime, InterpSpeed);
	NewLinearColor.A = FMath::FInterpConstantTo(CurrentAtmosphereColor.A, TargetAtmosphereColor.A, DeltaTime, InterpSpeed);

	return NewLinearColor;
}

// ==================== Environments ==================== //

void AWeatherController::PrepareEnvironments()
{
	// Thunder
	RainThunder = GetWorld()->SpawnActor<ARainThunder>(RainThunderClass);
	RainThunder->SetGlobalMatParamIns(GlobalMatParamIns.Get());
}
