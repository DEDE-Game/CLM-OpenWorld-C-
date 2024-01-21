// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFrameworks/EnemyController.h"
#include "Characters/EnemyCharacter.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"

AEnemyController::AEnemyController()
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

void AEnemyController::ReferencesInitializer()
{
    EnemyCharacter = Cast<AEnemyCharacter>(GetPawn());
}

// ==================== Lifecycles ==================== //

void AEnemyController::BeginPlay()
{
    Super::BeginPlay();

    // Events
    GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetSense);

    // ...
    ReferencesInitializer();
}

void AEnemyController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Strafing();
}

// ==================== AI ==================== //

void AEnemyController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    // Then attack if success
    bool bCanAttack = Result.IsSuccess() && EnemyCharacter->TargetCombat.IsValid();

    if (bCanAttack) Engage();
}

void AEnemyController::ActivateReaction()
{
   if (!EnemyCharacter->bEquipWeapon) EnemyCharacter->SwapWeapon(1);

    GetWorldTimerManager().SetTimer(
        ReactionDelay,
        this,
        &ThisClass::FinishedReaction,
        .8f
    );
}

void AEnemyController::OnTargetSense(AActor* Actor, FAIStimulus Stimulus)
{
    AOWCharacter* Other = Cast<AOWCharacter>(Actor);

    // If already have target...
    if (!Other || !EnemyCharacter->IsEnemy(Other) || GetWorldTimerManager().IsTimerActive(ReactionDelay)) return;
    
    if (!EnemyCharacter->bEquipWeapon) EnemyCharacter->SwapWeapon(1);

    EnemyCharacter->SetLockOn(Other);
}

// ==================== Combat ==================== //

void AEnemyController::CheckRange()
{
    if (!EnemyCharacter.IsValid() || !EnemyCharacter->TargetCombat.IsValid()) return;

    FVector CharacterLocation = EnemyCharacter->GetActorLocation();
    FVector TargetLocation    = EnemyCharacter->TargetCombat->GetActorLocation();

    float Distance = (TargetLocation - CharacterLocation).Size();

    if   (Distance <= HitRange) EnemyCharacter->Attack();
    else                        MoveToLocation(TargetLocation, 200.f, false);
}

void AEnemyController::FinishedReaction()
{
    EnemyCharacter->SetLockOn(EnemyCharacter->TargetCombat.Get());
}

void AEnemyController::Engage()
{
    // Reset
    bStrafing = false;

    // Get the decision randomly, but we can adjust the aggresivly
    int8 Decision = FMath::RandRange(0, EngagingChances.Num() - 1);

    switch (Decision)
    {
    case 0:
        CheckRange();
        break;

    case 1:
        bStrafing = true;
        StrafeDirectionX = FMath::RandRange(0, 1) ? -1.f : 1.f;
        StrafeDirectionY = FMath::RandRange(0, 1) ? -1.f : 1.f;
        break;

    case 2:
        EnemyCharacter->ToggleBlock(true);
        break;

    default:
        CheckRange();
        break;
    }

    // Randomize next decision
    float NextDecisionTimer = FMath::RandRange(EngageDelayMin, EngageDelayMax);
    GetWorldTimerManager().SetTimer(EngageDelayHandle, this, &ThisClass::Engage, NextDecisionTimer);
}

void AEnemyController::Strafing()
{
    if (!bStrafing || !EnemyCharacter.IsValid() || !EnemyCharacter->TargetCombat.IsValid()) return;

    FRotator Rotation = GetControlRotation();
    Rotation.Roll = Rotation.Pitch = 0.f;

    FVector RightVector = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
    FVector FrontVector = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);

    EnemyCharacter->AddMovementInput(RightVector, StrafeDirectionX);
    EnemyCharacter->AddMovementInput(FrontVector, StrafeDirectionY);
}
