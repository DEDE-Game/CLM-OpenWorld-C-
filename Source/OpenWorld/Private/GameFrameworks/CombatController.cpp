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
    bool bCanDecide = Result.IsSuccess() && CombatCharacter->TargetCombat.IsValid();

    // Start to Decide
    if (bCanDecide) Engage();
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
    bool bCantSense = bDisableSense || // OR
                      CombatCharacter->IsOnMontage("Stunned") || // OR
                      !Other        || // OR
                      !CombatCharacter->IsEnemy(Other) || // OR
                      GetWorldTimerManager().IsTimerActive(ReactionDelay);

    if (bCantSense) return;

    // Using weapon
    if (!CombatCharacter->bEquipWeapon) CombatCharacter->SwapWeapon();

    // ...
    Other          ->DeactivateAction();
    CombatCharacter->SetLockOn(Other);
}

void ACombatController::Engage()
{
    // Reset
    CombatCharacter->ToggleWalk(false);
    bStrafing     = false;
    bDisableSense = true;
    GetWorldTimerManager().ClearTimer(PatrollingDelayHandler);

    // Randomize next Engage
    float NextEngageTimer = FMath::RandRange(EngageDelayMin, EngageDelayMax);
    GetWorldTimerManager().SetTimer(EngageDelayHandle, this, &ThisClass::Engage, NextEngageTimer);

    // If on doing something, do none
    if (CombatCharacter->IsOnMontage()) return;

    // When the enemy is dead
    if (!CombatCharacter->TargetCombat.IsValid() || CombatCharacter->TargetCombat->IsDead())
    {
        // Check for nearest enemy first
        CombatCharacter->LockNearest();

        // If nearest enemy is still not exists, start patrolling
        if (!CombatCharacter->TargetCombat.IsValid())
        {
            StartPatrolling();

            return;
        }
    }

    // Get the decision randomly, but we can adjust the aggresivly
    int8 Decision = FMath::RandRange(0, EngageChances.Num() - 1);
    Decision = EngageChances[Decision];
    
    switch (Decision)
    {
    // Attacking
    case 0:
        Attacking();
        break;

    // Strafing
    case 1:
        StartStrafing();
        break;

    // Blocking
    case 2:
        Blocking();
        break;

    // Charge Attack
    case 3:
        CombatCharacter->StartChargeAttack();
        break;

    default:
        Attacking();
        break;
    }
}

void ACombatController::StartStrafing()
{
    bStrafing = true;
    StrafeDirectionX = FMath::RandRange(0, 1) ? -1.f : 1.f;
    StrafeDirectionY = FMath::RandRange(-1.f, 1.f);
}

void ACombatController::Blocking()
{
    CombatCharacter->ToggleBlock(true);

    // Disable it after certain time
    float Timer = FMath::RandRange(1.f, 4.f);

    GetWorldTimerManager().SetTimer(
        BlockingTimerHandle, [this]()
        { CombatCharacter->ToggleBlock(false); },
        Timer, false
    );
}

// ==================== Patrolling ==================== //

void ACombatController::StartPatrolling()
{
    // Unequip any weapon
    if (CombatCharacter->bEquipWeapon && (!CombatCharacter->TargetCombat.IsValid() || CombatCharacter->TargetCombat->IsDead()))
        CombatCharacter->SwapWeapon();

    CombatCharacter->SetLockOn(nullptr);
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

void ACombatController::Attacking()
{
    FVector CharacterLocation = CombatCharacter->GetActorLocation();
    FVector TargetLocation    = CombatCharacter->TargetCombat->GetActorLocation();

    float Distance = (TargetLocation - CharacterLocation).Size();

    // Make sure to use weapon first
    if (!CombatCharacter->bEquipWeapon) CombatCharacter->SwapWeapon();

    // If character is not ready to attack yet AND
    // Only attacking if the enemy's target is self instead, he will just strafing, blocking, etc
    bool bNotForceAttack = CombatCharacter->GetTargetCombat()->GetTargetCombat() /* Enemy's target combat */ != CombatCharacter &&
                        !FMath::RandBool();
    if (!CombatCharacter->IsReady() || bNotForceAttack)
    {
        ReEngage();

        return;
    }

    // Go to that target combat
    if (Distance > HitRange)
    {
        bDisableSense = false;

        CombatCharacter->LockNearest();
        MoveToLocation(TargetLocation, 250.f, false);

        return;
    }

    // The distance is close, so we can attack now
    // Try to kick enemy that is on blocking
    bool bShouldAttack = CombatCharacter->TargetCombat->IsBlocking() ? FMath::RandBool() : true;

    if (bShouldAttack) CombatCharacter->Attack();
    else               CombatCharacter->StartKick();
}

void ACombatController::FinishedReaction()
{
    CombatCharacter->SetLockOn(CombatCharacter->TargetCombat.Get());
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
