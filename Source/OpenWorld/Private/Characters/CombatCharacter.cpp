// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CombatCharacter.h"
#include "Characters/PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFrameworks/CombatController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationInvokerComponent.h"
#include "Weapons/MeleeWeapon.h"
#include "Widgets/HealthBar.h"

ACombatCharacter::ACombatCharacter()
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
    HealthBarComponent->SetVisibility(true);

    // Nav Invoker
    NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("Navigation Invoker"));

    // ...
    Team = ETeam::T_Enemy;
    DefaultInitializer();
}

void ACombatCharacter::DefaultInitializer()
{
    static ConstructorHelpers::FClassFinder<ACombatController> CombatControllerAsset(
        TEXT("/Script/OpenWorld.CombatController")
    );

    AIControllerClass = CombatControllerAsset.Class;

    static ConstructorHelpers::FClassFinder<UUserWidget> HealthBarAsset(
        TEXT("/Game/Game/Blueprints/Widgets/CombatCharacter/WBP_HealthBar")
    );
    HealthBarComponent->SetWidgetClass(HealthBarAsset.Class);

    static ConstructorHelpers::FClassFinder<AMeleeWeapon> AxeAsset(
        TEXT("/Game/Game/Blueprints/Weapons/BP_Axe")
    );
    GivenWeaponClasses.Add(AxeAsset.Class);

    static ConstructorHelpers::FClassFinder<AMeleeWeapon> SwordAsset(
        TEXT("/Game/Game/Blueprints/Weapons/BP_Sword")
    );
    GivenWeaponClasses.Add(SwordAsset.Class);

	static ConstructorHelpers::FObjectFinder<USoundBase> SlowSFXAsset(
		TEXT("/Script/MetasoundEngine.MetaSoundSource'/Game/Game/Audio/Combats/MS_Slow.MS_Slow'")
	);
	SlowSFX = SlowSFXAsset.Object;
}

// ==================== Lifecycles ==================== //

void ACombatCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Give weapon
    RandomizeWeapon();

    // Initializing UI
    HealthBar = Cast<UHealthBar>(HealthBarComponent->GetUserWidgetObject());
    HealthBar->UpdateHealth(Health / MaxHealth);
    if (Team == ETeam::T_Friend) HealthBar->SetHealthColor({ 0.548f, 0.973f, 0.162f, 1.f });
}

void ACombatCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    EnemyController = Cast<ACombatController>(NewController);
}

void ACombatCharacter::Destroyed()
{
    Super::Destroyed();

    if (EnemyController.IsValid()) EnemyController->Destroy();
}

// ==================== Attributes ==================== //

void ACombatCharacter::Die()
{
    Super::Die();

    HealthBarComponent->SetVisibility(false);
}

// ==================== Combat ==================== //

void ACombatCharacter::RandomizeWeapon()
{
    int8 RandomWeapon = FMath::RandRange(0, GivenWeaponClasses.Num() - 1);

    CarriedWeapon = GetWorld()->SpawnActor<AMeleeWeapon>(GivenWeaponClasses[RandomWeapon]);
    CarriedWeapon->Pickup(this, TEXT("Back0 Socket"));
}

void ACombatCharacter::OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& HitImpact, const float GivenDamage, bool bBlockable)
{
    Super::OnWeaponHit(DamagingCharacter, HitImpact, GivenDamage, bBlockable);

    // Make the enemy lock to that damaging actor with delay to give a time for hit reaction
    if (!TargetCombat.IsValid() || DamagingCharacter != TargetCombat.Get())
    {
        TargetCombat = DamagingCharacter;

        // Only delay reaction if the damage is not insta
        if (GivenDamage < Health) EnemyController->ActivateReaction();
    }

    // Update UI
    HealthBar->UpdateHealth(Health / MaxHealth);
}

void ACombatCharacter::Attack()
{
    Super::Attack();

    // Slow down the time to give player a chance to dodge/block
    if (TargetCombat.IsValid() && TargetCombat->GetClass()->IsChildOf(APlayerCharacter::StaticClass()))
    {
        UGameplayStatics::PlaySound2D(this, SlowSFX.LoadSynchronous());
        UGameplayStatics::SetGlobalTimeDilation(this, .5f);
    }
}

void ACombatCharacter::EnableWeapon(bool bEnabled)
{
    Super::EnableWeapon(bEnabled);

    // After attacking, set back the time to normal
    if (TargetCombat.IsValid() && TargetCombat->GetClass()->IsChildOf(APlayerCharacter::StaticClass()))
        UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
}

void ACombatCharacter::SwapWeapon()
{
    bEquipWeapon = !bEquipWeapon;
    
    Super::SwapWeapon();
}

void ACombatCharacter::SetLockOn(AOWCharacter* Target)
{
    Super::SetLockOn(Target);

    if (!Target) return;
    EnemyController->MoveToLocation(Target->GetActorLocation(), 250.f, false);
}
