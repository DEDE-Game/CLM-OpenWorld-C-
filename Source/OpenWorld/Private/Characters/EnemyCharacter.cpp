// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFrameworks/EnemyController.h"
#include "NavigationInvokerComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Weapons/Weapon.h"

AEnemyCharacter::AEnemyCharacter()
{
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Make combo over for enemy longer
    ComboOverTimer = 5.f;

    // Collision
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()            ->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    // Nav Invoker
    NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("Navigation Invoker"));

    // ...
    Team = ETeam::T_Enemy;
    DefaultInitializer();
}

void AEnemyCharacter::DefaultInitializer()
{
    static ConstructorHelpers::FClassFinder<AEnemyController> EnemyControllerAsset(
        TEXT("/Script/OpenWorld.EnemyController")
    );

    AIControllerClass = EnemyControllerAsset.Class;
}

// ==================== Lifecycles ==================== //

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Give weapon
    CarriedWeapon = GetWorld()->SpawnActor<AWeapon>(GivenWeapon);
    CarriedWeapon->EquipTo(this, TEXT("Back Socket"));
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    EnemyController = Cast<AEnemyController>(NewController);
}

void AEnemyCharacter::Destroyed()
{
    Super::Destroyed();

    if (EnemyController.IsValid()) EnemyController->Destroy();
}

// ==================== Combat ==================== //

void AEnemyCharacter::OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& HitImpact, const float GivenDamage)
{
    Super::OnWeaponHit(DamagingCharacter, HitImpact, GivenDamage);

    if (!TargetCombat.IsValid())
    {
        TargetCombat = DamagingCharacter;

        // Make the enemy lock to that damaging actor with delay to give a time for hit reaction
        EnemyController->ActivateReaction();
    }
}

void AEnemyCharacter::SetLockOn(AOWCharacter* Target)
{
    if (!Target) return;

    Super::SetLockOn(Target);

    EnemyController->MoveToLocation(Target->GetActorLocation(), 200.f, false);
}
