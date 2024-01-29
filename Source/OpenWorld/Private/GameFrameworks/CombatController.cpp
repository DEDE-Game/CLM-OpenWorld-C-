// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFrameworks/CombatController.h"
#include "Characters/CombatCharacter.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

ACombatController::ACombatController()
{
    PrimaryActorTick.bCanEverTick = true;

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

void ACombatController::ReferencesInitializer()
{
    CombatCharacter = Cast<ACombatCharacter>(GetPawn());
}

// ==================== Lifecycles ==================== //

void ACombatController::BeginPlay()
{
    Super::BeginPlay();

    // Events
    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetSense);

    // ...
    ReferencesInitializer();
    StartPatrolling();
}

void ACombatController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Strafing();
}

// ==================== AI ==================== //

void ACombatController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    // Then attack if success
    bool bCanAttack = Result.IsSuccess() && CombatCharacter->TargetCombat.IsValid();

    // Start to attack
    if (bCanAttack) Engage();
    else
    {
        // Investigate then start to patrolling
        float Timer = FMath::RandRange(PatrollingDelayMin, PatrollingDelayMax);

        GetWorldTimerManager().SetTimer(
            PatrollingDelayHandler,
            this,
            &ThisClass::StartPatrolling,
            Timer
        );
    }
}

void ACombatController::ActivateReaction()
{
   if (!CombatCharacter->bEquipWeapon) CombatCharacter->SwapWeapon();

    GetWorldTimerManager().SetTimer(
        ReactionDelay,
        this,
        &ThisClass::FinishedReaction,
        .8f
    );
}

void ACombatController::OnTargetSense(AActor* Actor, FAIStimulus Stimulus)
{
    AOWCharacter* Other = Cast<AOWCharacter>(Actor);

    // If already have target...
    if (bDisableSense || !Other || !CombatCharacter->IsEnemy(Other) || GetWorldTimerManager().IsTimerActive(ReactionDelay)) return;

    // Using weapon
    if (!CombatCharacter->bEquipWeapon) CombatCharacter->SwapWeapon();

    // ...
    Other          ->DeactivateAction();
    CombatCharacter->SetLockOn(Other);
}

// ==================== Patrolling ==================== //

void ACombatController::StartPatrolling()
{
    // Unequip any weapon
    if (CombatCharacter->bEquipWeapon && !CombatCharacter->TargetCombat.IsValid())
        CombatCharacter->SwapWeapon();

    CombatCharacter->ToggleWalk(true);

    // Make sure to re-enable sense
    bDisableSense = false;

    // Pick random place
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation NavLocationResult;

    NavSystem->GetRandomReachablePointInRadius(CombatCharacter->GetActorLocation(), 1500.f, NavLocationResult);

    MoveToLocation(NavLocationResult.Location);
}

// ==================== Combat ==================== //

void ACombatController::CheckRange()
{
    if (!CombatCharacter.IsValid() || !CombatCharacter->TargetCombat.IsValid())
    {
        StartPatrolling();

        return;
    }

    FVector CharacterLocation = CombatCharacter->GetActorLocation();
    FVector TargetLocation    = CombatCharacter->TargetCombat->GetActorLocation();

    float Distance = (TargetLocation - CharacterLocation).Size();

    if   (Distance <= HitRange) CombatCharacter->Attack();
    else
    {
        bDisableSense = false;

        CombatCharacter->LockNearest();
        MoveToLocation(TargetLocation, 250.f, false);
    }
}

void ACombatController::FinishedReaction()
{
    CombatCharacter->SetLockOn(CombatCharacter->TargetCombat.Get());
}

void ACombatController::Engage()
{
    if (!CombatCharacter->TargetCombat.IsValid())
    {
        StartPatrolling();

        return;
    }

    // Reset
    CombatCharacter->ToggleWalk(false);
    bStrafing     = false;
    bDisableSense = true;

    // Get the decision randomly, but we can adjust the aggresivly
    int8 Decision = FMath::RandRange(0, EngagingChances.Num() - 1);
    Decision = EngagingChances[Decision];

    switch (Decision)
    {
    // Attacking
    case 0:
        CheckRange();
        break;

    // Strafing
    case 1:
        bStrafing = true;
        StrafeDirectionX = FMath::RandRange(0, 1) ? -1.f : 1.f;
        StrafeDirectionY = FMath::RandRange(-1.f, 1.f);
        break;

    // Blocking
    case 2:
    {
        CombatCharacter->ToggleBlock(true);

        // Disable it after certain time
        float Timer = FMath::RandRange(1.f, 4.f);

        GetWorldTimerManager().SetTimer(BlockingTimerHandle, [this]() {
            CombatCharacter->ToggleBlock(false);
        }, Timer, false);
        break;
    }

    default:
        CheckRange();
        break;
    }

    // Randomize next decision
    float NextDecisionTimer = FMath::RandRange(EngageDelayMin, EngageDelayMax);
    GetWorldTimerManager().SetTimer(EngageDelayHandle, this, &ThisClass::Engage, NextDecisionTimer);
}

void ACombatController::Strafing()
{
    if (!bStrafing || !CombatCharacter.IsValid()) return;

    FRotator Rotation = GetControlRotation();
    Rotation.Roll = Rotation.Pitch = 0.f;

    FVector RightVector = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
    FVector FrontVector = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);

    CombatCharacter->AddMovementInput(RightVector, StrafeDirectionX);
    CombatCharacter->AddMovementInput(FrontVector, StrafeDirectionY);
}
