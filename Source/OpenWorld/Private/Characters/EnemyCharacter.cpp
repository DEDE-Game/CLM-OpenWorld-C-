// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFrameworks/EnemyController.h"
#include "NavigationInvokerComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Weapons/Weapon.h"
#include "Widgets/HealthBar.h"

AEnemyCharacter::AEnemyCharacter()
{
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Make combo over for enemy longer
    ComboOverTimer = 5.f;

    // Collision
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()            ->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    // Health Bar
    HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Bar"));
    HealthBarComponent->SetupAttachment(RootComponent);
    HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
    HealthBarComponent->SetDrawAtDesiredSize(true);

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

    static ConstructorHelpers::FClassFinder<UUserWidget> HealthBarAsset(
        TEXT("/Game/Game/Blueprints/Widgets/Enemy/WBP_HealthBar")
    );
    HealthBarComponent->SetWidgetClass(HealthBarAsset.Class);
}

// ==================== Lifecycles ==================== //

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Give weapon
    CarriedWeapon = GetWorld()->SpawnActor<AWeapon>(GivenWeapon);
    CarriedWeapon->EquipTo(this, TEXT("Back Socket"));

    // Initializing UI
    HealthBar = Cast<UHealthBar>(HealthBarComponent->GetUserWidgetObject());
    HealthBar->UpdateHealth(Health / MaxHealth);
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

    // Make the enemy lock to that damaging actor with delay to give a time for hit reaction
    if (!TargetCombat.IsValid())
    {
        TargetCombat = DamagingCharacter;

        // Only delay reaction if the damage is not insta
        if (GivenDamage < Health) EnemyController->ActivateReaction();
    }

    // Update UI
    HealthBar->UpdateHealth(Health / MaxHealth);
}

void AEnemyCharacter::SetLockOn(AOWCharacter* Target)
{
    Super::SetLockOn(Target);

    if (!Target) return;
    EnemyController->MoveToLocation(Target->GetActorLocation(), 200.f, false);
}

void AEnemyCharacter::Die()
{
    Super::Die();

    HealthBarComponent->SetVisibility(false);
}
