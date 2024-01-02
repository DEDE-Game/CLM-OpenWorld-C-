// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/DaytimeController.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"

UDaytimeController::UDaytimeController()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = .05f;

}

void UDaytimeController::ReferencesInitializer()
{
	// Get all actors
	TArray<AActor*> DirectionalLights;
	UGameplayStatics::GetAllActorsOfClass(this, ADirectionalLight::StaticClass(), DirectionalLights);

	// Store it as references
	Sun  = Cast<ADirectionalLight>(DirectionalLights[0]);
	Moon = Cast<ADirectionalLight>(DirectionalLights[1]);
}

// ==================== Lifecycles ==================== //

void UDaytimeController::BeginPlay()
{
	Super::BeginPlay();

	ReferencesInitializer();
	UpdateTime();
}

void UDaytimeController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTime();
}

// ==================== Lightings ==================== //

void UDaytimeController::UpdateTime()
{
	// Update timer
	Time = Time + TimeRate;

	if (Time > 24.f) Time = 0.f;

	// Move the sun by Rotating it based on the time
	float Rotation = Time * (360.f / 24.f);
	float Day	   = Rotation + 90.f;
	float Night    = Rotation - 270.f;
	float SunPitch = Time > 18.f ? Night : Day;
	
	Sun ->SetActorRotation(FRotator(SunPitch, 0.f, 0.f));

	// Adjust moon/sun affecting world
	bool bShouldAdjust = Time < 6.f || Time > 18.f;

	Sun ->GetComponent()->SetVisibility(!bShouldAdjust);
	Moon->GetComponent()->SetVisibility(bShouldAdjust);
}
