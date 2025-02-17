// Fill out your copyright notice in the Description page of Project Settings.

#include "Managers/DaytimeManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADaytimeManager::ADaytimeManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = .5f;

#if WITH_EDITORONLY_DATA
	bIsSpatiallyLoaded = false;
#endif

	// Root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent->PrimaryComponentTick.bCanEverTick = false;

	// Sun Scene
	SunScene = CreateDefaultSubobject<USceneComponent>(TEXT("Sun Scene"));
	SunScene->PrimaryComponentTick.bCanEverTick = false;
	SunScene->SetupAttachment(RootComponent);
	SunScene->SetRelativeLocation({ 0.f, 200.f, 0.f });

	// Moon Mesh
	MoonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Moon Mesh"));
	MoonMesh->PrimaryComponentTick.bCanEverTick = false;
	MoonMesh->SetupAttachment(SunScene);
	MoonMesh->SetRelativeLocation({ 0.f, -2800000.f, 0.f });
	MoonMesh->SetRelativeScale3D({ 2500.f, 2500.f, 2500.f });
	MoonMesh->SetCastShadow(false);
	MoonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	MoonMesh->SetStaticMesh(SphereMeshAsset.Object);
}

void ADaytimeManager::ReferencesInitializer()
{
	// Get all actors
	TArray<AActor*> DirectionalLights;
	UGameplayStatics::GetAllActorsOfClass(this, ADirectionalLight::StaticClass(), DirectionalLights);

	// Store it as references
	if (!DirectionalLights.IsEmpty())
	{
		Sun  = !Sun.IsValid()  ? Cast<ADirectionalLight>(DirectionalLights[0]) : Sun;
		Moon = !Moon.IsValid() ? Cast<ADirectionalLight>(DirectionalLights[1]) : Moon;
	}
}

// ==================== Lifecycles ==================== //

void ADaytimeManager::OnConstruction(const FTransform& Transform)
{
	ReferencesInitializer();
	UpdateTime();
}

void ADaytimeManager::BeginPlay()
{
	Super::BeginPlay();

}

void ADaytimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTime();
}

// ==================== Lightings ==================== //

void ADaytimeManager::UpdateTime()
{
	if (!Sun.IsValid() || !Moon.IsValid()) return;

	// Update timer
	Time = Time + TimeRate;

	if (Time > 24.f) Time = 0.f;

	// Calculate sun's angle based on the time
	float Rotation = Time * (360.f / 24.f);
	float Day	   = Rotation + 90.f;
	float Night    = Rotation - 270.f;
	float Angle    = Time > 18.f ? Night : Day;

	// Gimble Lock fix
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FRotator(0.f, CurrentRotation.Yaw, Angle));

	FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(
		SunScene->GetComponentLocation(),
		GetActorLocation()
	);
	Sun->SetActorRotation(NewRotation);

	// Adjust moon/sun affecting world
	bool bShouldAdjust = Time < 5.5f || Time > 17.5f;

	Sun ->GetComponentByClass<UDirectionalLightComponent>()->SetVisibility(!bShouldAdjust);
	Moon->GetComponentByClass<UDirectionalLightComponent>()->SetVisibility(bShouldAdjust);
	MoonMesh->SetHiddenInGame(!bShouldAdjust);
	MoonMesh->SetVisibility(bShouldAdjust);
}
