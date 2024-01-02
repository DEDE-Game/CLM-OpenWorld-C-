// Fill out your copyright notice in the Description page of Project Settings.

#include "Environments/RainThunder.h"
#include "Components/AudioComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ARainThunder::ARainThunder()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent->SetMobility(EComponentMobility::Movable);

	// Plane Mesh
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane Mesh"));
	PlaneMesh->PrimaryComponentTick.bCanEverTick 		  = false;
	PlaneMesh->PrimaryComponentTick.bStartWithTickEnabled = false;
	PlaneMesh->SetupAttachment(RootComponent);
	PlaneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlaneMesh->SetMobility(EComponentMobility::Movable);
	PlaneMesh->SetRelativeScale3D(FVector(20.f));
	PlaneMesh->SetRelativeRotation(FRotator(0.f, -90.f, 90.f));

	// Thunder Timeline
	ThunderTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Thunder Timeline"));
	ThunderTimeline->SetLooping(false);
	ThunderTimeline->SetTimelineLength(.88f);

	// ...
	DefaultInitializer();
}

void ARainThunder::DefaultInitializer()
{
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> RainAsset(
		TEXT("/Script/Niagara.NiagaraSystem'/Game/Game/VFX/NS_Rain.NS_Rain'")
	);
	RainVFX = RainAsset.Object;

	ConstructorHelpers::FObjectFinder<UMaterialInterface> ParentThunderMatAsset(
		TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Game/Materials/Skies/MI_Thunder.MI_Thunder'")
	);
	ParentThunderMaterial = ParentThunderMatAsset.Object;

	ConstructorHelpers::FObjectFinder<USoundBase> RainSFXAsset(
		TEXT("/Script/Engine.SoundWave'/Game/Game/Audio/Thunders/LightRain.LightRain'")
	);
	RainSFX = RainSFXAsset.Object;

	ConstructorHelpers::FObjectFinder<UCurveFloat> ThunderCurveAsset(
		TEXT("/Script/Engine.CurveFloat'/Game/Game/Curves/C_ThunderCurve.C_ThunderCurve'")
	);
	ThunderCurve = ThunderCurveAsset.Object;

	ConstructorHelpers::FObjectFinder<USoundBase> ThunderSFXAsset(
		TEXT("/Script/MetasoundEngine.MetaSoundSource'/Game/Game/Audio/Thunders/MS_Thunders.MS_Thunders'")
	);
	ThunderSFX = ThunderSFXAsset.Object;
}

// ==================== Lifecycles ==================== //

void ARainThunder::BeginPlay()
{
	Super::BeginPlay();
	
	CreateMaterial();
	SetupThunder();
}

void ARainThunder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRain(DeltaTime);

	// Watch for any progress, if none disable
	if (!bRaining) SetActorTickEnabled(false);
}

// ==================== Materials ==================== //

void ARainThunder::CreateMaterial()
{
	if (!ParentThunderMaterial.IsValid()) return;

	ThunderMaterial = UMaterialInstanceDynamic::Create(ParentThunderMaterial.LoadSynchronous(), this);
	PlaneMesh->SetMaterial(0, ThunderMaterial);
}

// ==================== Thunder ==================== //

void ARainThunder::SetupThunder()
{
	// Setup timeline
	FOnTimelineFloatStatic OnTimelineCallback;	
	OnTimelineCallback.BindUObject(this, &ThisClass::UpdateThunderTimeline);

	ThunderTimeline->AddInterpFloat(ThunderCurve.LoadSynchronous(), OnTimelineCallback);

	// Setup post process volume
	PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(this, APostProcessVolume::StaticClass()));
	PostProcessVolume->Settings.bOverride_AutoExposureBias   = true;

	// Get the player pawn
	PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

void ARainThunder::UpdateThunderTimeline(float Value)
{
	ThunderMaterial->SetScalarParameterValue(TEXT("Y"), Value);
	ThunderMaterial->SetScalarParameterValue(TEXT("Emission"), (Value + 1.f) * 50.f);
}

void ARainThunder::Relocate()
{
	FRotator ControllerRotation = PlayerPawn->GetControlRotation();

	// Make the thunder infront of the player
	const FVector ForwardDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X) * FMath::RandRange(5000.f, 8000.f);
	const FVector RightDirection   = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::Y);
	const FVector PlayerLocation   = PlayerPawn->GetActorLocation();

	FVector ThunderLocation = ForwardDirection + PlayerLocation + RightDirection * FMath::RandRange(-8000.f, 8000.f) + FVector(0.f, 0.f, 3000.f);

	// Make the thunder always facing the player
	float RotationYaw = UKismetMathLibrary::FindLookAtRotation(ThunderLocation, PlayerLocation).Yaw;

	SetActorLocationAndRotation(ThunderLocation, FRotator(0.f, RotationYaw, 0.f));
}

void ARainThunder::Flashing()
{
	// Update latest expose info so it wont be bugging for the lighting
	DefaultExposureBias = PostProcessVolume->Settings.AutoExposureBias;

	// ...
	FTimerHandle FlashTimer;
	GetWorldTimerManager().SetTimer(FlashTimer, [this] { 
		PostProcessVolume->Settings.AutoExposureBias = 9.f;
	}, .2f, false);

	FTimerHandle NormalTimer;
	GetWorldTimerManager().SetTimer(NormalTimer, [this] {
		PostProcessVolume->Settings.AutoExposureBias = DefaultExposureBias;
		SetActorHiddenInGame(true);

		// Play thunder sounds, as in real life sound's speed is slower than light
		UGameplayStatics::PlaySoundAtLocation(this, ThunderSFX.LoadSynchronous(), GetActorLocation());
	}, .4f, false);
}

// ==================== Rain ==================== //

void ARainThunder::UpdateRain(float DeltaTime)
{
	if (!bRaining) return;

	// Updating rain weather
	RainLevel = FMath::Clamp(RainLevel + ChangeRate * DeltaTime, 0.f, .8f);
	GlobalMatParamIns->SetScalarParameterValue(TEXT("RainLevel"), RainLevel);
	
	if (RainLevel == 0.f || RainLevel == .8f) bRaining = false;
}

// ==================== Interfaces ==================== //

void ARainThunder::Strike()
{
	Relocate();

	// Pick random texture for variation purpose
	int8 RandomText = FMath::RandRange(0, ThunderTextures.Num() - 1);
	ThunderMaterial->SetTextureParameterValue(TEXT("ThunderTexture"), ThunderTextures[RandomText].LoadSynchronous());

	// Play timeline
	SetActorHiddenInGame(false);
	ThunderTimeline->PlayFromStart();

	Flashing();
}

void ARainThunder::ToggleThunder(bool bEnabled)
{
	if (bEnabled)
	{
		float RandomTimer = FMath::RandRange(ThunderTimerMin, ThunderTimerMax);

		GetWorldTimerManager().SetTimer(ThunderTimerHandle, this, &ThisClass::Strike, RandomTimer, true);
	}
	else
		GetWorldTimerManager().ClearTimer(ThunderTimerHandle);
}

void ARainThunder::ToggleRain(bool bEnabled)
{
	SetActorTickEnabled(true);

	if (bEnabled && PlayerPawn.IsValid())
	{
		// Attach the rain to player
		RainComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			RainVFX.LoadSynchronous(),
			PlayerPawn->GetRootComponent(),
			FName(),
			FVector(0.f, 0.f, 2500.f),
			FRotator(0.f),
			EAttachLocation::KeepRelativeOffset,
			true
		);
		
		RainComponent->Activate();

		// Make everything wet, @see UpdateRain function
		bRaining   = true;
		ChangeRate = .05f;

		// Play sound
		RainSound = UGameplayStatics::SpawnSound2D(this, RainSFX.LoadSynchronous());
		RainSound->Play();
		RainSound->FadeIn(3.f);
	}
	else
	{
		// Make everything dry, @see UpdateRain function
		bRaining   = true;
		ChangeRate = -.05f;
		RainComponent->DestroyComponent();
		RainSound->FadeOut(3.f, 1.f);
		
		// Stop the rain sound
		FTimerHandle StopRainSound;
		GetWorldTimerManager().SetTimer(StopRainSound, [this]() { RainSound->Stop(); UE_LOG(LogTemp, Warning, TEXT("RainSound valid? %s"), RainSound ? TEXT("Yes") : TEXT("No")); }, 3.f, false);
	}
}

// ==================== Modifiers ==================== //

void ARainThunder::SetGlobalMatParamIns(UMaterialParameterCollectionInstance* InGlobalParam)
{
	GlobalMatParamIns = InGlobalParam;
}

