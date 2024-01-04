// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/DaytimeController.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADaytimeController::ADaytimeController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = .15f;
	bIsSpatiallyLoaded = false;

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
	MoonMesh->SetRelativeLocation({ 0.f, -10000000.f, 0.f });
	MoonMesh->SetRelativeScale3D({ 10000.f, 10000.f, 10000.f });
	MoonMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	MoonMesh->SetStaticMesh(SphereMeshAsset.Object);
}

void ADaytimeController::ReferencesInitializer()
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

void ADaytimeController::OnConstruction(const FTransform& Transform)
{
	ReferencesInitializer();
	UpdateTime();
}

void ADaytimeController::BeginPlay()
{
	Super::BeginPlay();

}

void ADaytimeController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTime();
}

// ==================== Lightings ==================== //

void ADaytimeController::UpdateTime()
{
	if (!Sun.IsValid() || !Moon.IsValid()) return;

	// Update timer
	Time = Time + TimeRate;

	if (Time > 24.f) Time = 0.f;

	// Calculate sun's angle based on the time
	float Rotation = Time * (360.f / 24.f);
	float Day	   = Rotation + 90.f;
	float Night    = Rotation - 270.f;
	float Angle = Time > 18.f ? Night : Day;

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

	Sun ->GetComponent()->SetVisibility(!bShouldAdjust);
	Moon->GetComponent()->SetVisibility(bShouldAdjust);
}
