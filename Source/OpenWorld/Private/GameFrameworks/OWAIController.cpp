// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFrameworks/OWAIController.h"
#include "Characters/EnemyCharacter.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "OWAIController.h"

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
    EnemyCharacter = Cast<AEnemyCharacter>(GetPawn());
}

// ==================== Lifecycles ==================== //

void AOWAIController::BeginPlay()
{
    Super::BeginPlay();

    // Events
    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetSense);

    // ...
    ReferencesInitializer();
}

// ==================== AI ==================== //

void AOWAIController::GoTo(const FVector& Location)
{
    EPathFollowingRequestResult::Type Result = MoveToLocation(Location, 200.f, false);

    // Then attack if success
    bool bCanAttack = Result != EPathFollowingRequestResult::Failed && EnemyCharacter->TargetCombat.IsValid();

    if (bCanAttack) PerformAttack();
}

void AOWAIController::ActivateReaction()
{
    GetWorldTimerManager().SetTimer(
        ReactionDelay,
        this,
        &ThisClass::FinishedReaction,
        .5f
    );
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

    FVector Vector    = InResult.Location - EnemyCharacter->GetActorLocation();
    FVector Direction = Vector.GetSafeNormal();
    float Distance    = Vector.Length();

    if (Distance < 2500.f) return InResult.Location;

    // When the target is out of radius
    FNavLocation OutResult;

    NavSystem->GetRandomReachablePointInRadius(
        EnemyCharacter->GetActorLocation() + Direction * 1500.f,
        100.f,
        OutResult
    );

    return OutResult.Location;
}

// ==================== Perceptions ==================== //

void AOWAIController::OnTargetSense(AActor* Actor, FAIStimulus Stimulus)
{
    AOWCharacter* Other = Cast<AOWCharacter>(Actor);

    // If already have target...
    if (!Other || !EnemyCharacter->IsEnemy(Other) || GetWorldTimerManager().IsTimerActive(ReactionDelay)) return;
    
    EnemyCharacter->SetLockOn(Other);
}

// ==================== Combat ==================== //

void AOWAIController::CheckRange()
{
    FVector CharacterLocation = EnemyCharacter->GetActorLocation();
    FVector TargetLocation    = EnemyCharacter->TargetCombat->GetActorLocation();

    float Distance = (TargetLocation - CharacterLocation).Size();

    if   (Distance <= HitRange)
    {
        PerformAttack();
        EnemyCharacter->Attack();
    } 
    else GoTo(TargetLocation);
}

void AOWAIController::PerformAttack()
{
    float AttackTimer = FMath::RandRange(AttackDelayMin, AttackDelayMax);
    GetWorldTimerManager().SetTimer(AttackDelayHandler, this, &ThisClass::CheckRange, AttackTimer);
}

void AOWAIController::FinishedReaction()
{
    EnemyCharacter->SetLockOn(EnemyCharacter->TargetCombat.Get());
}
