// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFrameworks/OWAIController.h"
#include "NavigationInvokerComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Weapons/Weapon.h"

AEnemyCharacter::AEnemyCharacter()
{
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
    static ConstructorHelpers::FClassFinder<AAIController> AIControllerAsset(
        TEXT("/Script/OpenWorld.OWAIController")
    );

    AIControllerClass = AIControllerAsset.Class;
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

    AIController = Cast<AOWAIController>(NewController);
}

// ==================== Combat ==================== //

void AEnemyCharacter::OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& HitImpact)
{
    Super::OnWeaponHit(DamagingCharacter, HitImpact);

    if (!TargetCombat.IsValid())
    {
        TargetCombat = DamagingCharacter;

        // Make the enemy lock to that damaging actor with delay to give a time for hit reaction
        AIController->ActivateReaction();
    }
}

void AEnemyCharacter::SetLockOn(AOWCharacter* Target)
{
    Super::SetLockOn(Target);

    if (!bEquipWeapon) SwapWeapon(1);
    
    AIController->GoTo(Target->GetActorLocation());
}
