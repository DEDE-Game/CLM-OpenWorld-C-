// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFrameworks/OWAIController.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

AOWAIController::AOWAIController()
{
    // Perception
    UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));

    /** Sight Perception */
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sense Config"));
    SightConfig->SightRadius = 3000.f;
    SightConfig->LoseSightRadius = 3500.f;
    SightConfig->PeripheralVisionAngleDegrees = 45.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    
    Perception->ConfigureSense(*SightConfig);

    /** Hearing Perception */
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
    HearingConfig->HearingRange = 3000.f;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

    Perception->ConfigureSense(*HearingConfig);
    Perception->SetDominantSense(UAISense_Sight::StaticClass());

    SetPerceptionComponent(*Perception);
}

void AOWAIController::ReferencesInitializer()
{
    ControlledAI = Cast<ACharacter>(GetPawn());
}

// ==================== Lifecycles ==================== //

void AOWAIController::BeginPlay()
{
    Super::BeginPlay();

    // ...
    ReferencesInitializer();
}

// ==================== AI ==================== //

void AOWAIController::GoTo(const FVector& Location)
{
    FVector FinalTarget = DetermineTargetLocation(Location);

    FAIMoveRequest Request;
    Request.SetGoalLocation(FinalTarget);
    Request.SetAcceptanceRadius(200.f);

    FPathFollowingRequestResult Result = MoveTo(Request);
    bool bSuccess = Result.Code == EPathFollowingRequestResult::RequestSuccessful;
UE_LOG(LogTemp, Warning, TEXT("Target: %s"), bSuccess?TEXT("Yes"):TEXT("No"));
    if (!bSuccess)
    {
        FVector Vector = (Location - ControlledAI->GetActorLocation());
        float Distance = Vector.Length();

        if (Distance >= 200.f) GoTo(FinalTarget);
    }
}

FVector AOWAIController::DetermineTargetLocation(const FVector& Target)
{
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

    // When the target is in radius
    FNavLocation InResult;

    NavSystem->GetRandomReachablePointInRadius(
        Target,
        100.f,
        InResult
    );

    FVector Vector    = InResult.Location - ControlledAI->GetActorLocation();
    FVector Direction = Vector.GetSafeNormal();
    float Distance    = Vector.Length();

    if (Distance < 2500.f) return InResult.Location;

    // When the target is out of radius
    FNavLocation OutResult;

    NavSystem->GetRandomReachablePointInRadius(
        ControlledAI->GetActorLocation() + Direction * 1500.f,
        100.f,
        OutResult
    );

    return OutResult.Location;
}
