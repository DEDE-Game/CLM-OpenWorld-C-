// Fill out your copyright notice in the Description page of Project Settings.

#include "DevelopmentUtils/HumanoidReference.h"
#include "Components/InstancedStaticMeshComponent.h"

AHumanoidReference::AHumanoidReference()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Default Root Component"));
	RootComponent->SetMobility(EComponentMobility::Static);

	// Dummy Reference
	DummyReference = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Dummy Reference"));
	DummyReference->SetupAttachment(RootComponent);
	DummyReference->SetMobility(EComponentMobility::Static);
}

// ==================== Generating ==================== //

void AHumanoidReference::AddHumanoidReference()
{
	// Clear all instances
	DummyReference->ClearInstances();

	// Place the humanoid
	FVector2D Size = GridSize / 2.f / Gap;
	for (int32 X = Size.X * -1.f; X <= Size.X; ++X)
		for (int32 Y = Size.Y * -1.f; Y <= Size.Y; ++Y)
		{
			FHitResult HitResult;

			GetWorld()->LineTraceSingleByChannel(
				HitResult,
				FVector(X * Gap, Y * Gap, TraceHeight),
				FVector(X * Gap, Y * Gap, TraceHeight * -1.f),
				ECollisionChannel::ECC_Visibility
			);

			// Finally spawn the dummy humaniod
			if (HitResult.bBlockingHit)
			{
				FTransform DummyTransform;
				DummyTransform.SetLocation(HitResult.ImpactPoint);

				DummyReference->AddInstance(
					DummyTransform,
					true
				);
			}
		}
}

void AHumanoidReference::ClearHumanoidRefenrence()
{
	DummyReference->ClearInstances();
}
