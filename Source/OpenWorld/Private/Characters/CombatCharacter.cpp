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

    // Combat
    ChargeAfter     = .05f;
    AutoChargeTimer = .4f;
    ComboOverTimer  = 5.f;

    // Collision
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetMesh()            ->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    // Health Bar
    HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Bar"));
    HealthBar->SetupAttachment(RootComponent);
    HealthBar->SetWidgetSpace(EWidgetSpace::Screen);
    HealthBar->SetDrawAtDesiredSize(true);
    HealthBar->SetVisibility(true);

    // Attack Indicator
    AttackIndicator = CreateDefaultSubobject<UWidgetComponent>(TEXT("Attack Indicator"));
    AttackIndicator->SetupAttachment(RootComponent);
    AttackIndicator->SetWidgetSpace(EWidgetSpace::Screen);
    AttackIndicator->SetDrawAtDesiredSize(true);
    AttackIndicator->SetVisibility(false);

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
    HealthBar->SetWidgetClass(HealthBarAsset.Class);

    static ConstructorHelpers::FClassFinder<UUserWidget> AttackIndicatorAsset(
        TEXT("/Game/Game/Blueprints/Widgets/CombatCharacter/WBP_AttackIndicator")
    );
    AttackIndicator->SetWidgetClass(AttackIndicatorAsset.Class);

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

void ACombatCharacter::InitializeUI()
{
    HealthBarWidget = Cast<UHealthBar>(HealthBar->GetUserWidgetObject());
    HealthBarWidget->UpdateHealth(Health / MaxHealth);

    if (Team == ETeam::T_Friend)
    {
        HealthBarWidget->SetHealthColor({0.548f, 0.973f, 0.162f, 1.f});

        // Destroy attack indicator as we don't need it
        AttackIndicator->DestroyComponent();
    }
}

// ==================== Lifecycles ==================== //

void ACombatCharacter::BeginPlay()
{
    Super::BeginPlay();

    RandomizeWeapon();
    InitializeUI();
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

    HealthBar->SetVisibility(false);
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
    HealthBarWidget->UpdateHealth(Health / MaxHealth);
}

void ACombatCharacter::AttackCombo()
{
    Super::AttackCombo();

    // Give player chance to dodge/block
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(TargetCombat.Get()))
    {
        AttackIndicator->SetVisibility(true);
        PlayerCharacter->ShowTip(TEXT("[ALT] / [RMB] - To Dodge / Block"));
    }
}

void ACombatCharacter::StartChargeAttack()
{
    Super::StartChargeAttack();

    // Give player chance to dodge since charge attack is unblockable
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(TargetCombat.Get()))
    {
        // Attack Indicator
        AttackIndicator->SetVisibility(true);
        PlayerCharacter->ShowTip(TEXT("[ALT] + [WASD] - To dash"));

        // Slow the time
        UGameplayStatics::PlaySound2D(this, SlowSFX.LoadSynchronous());
        UGameplayStatics::SetGlobalTimeDilation(this, .5f);
    }
}

void ACombatCharacter::EnableWeapon(bool bEnabled)
{
    Super::EnableWeapon(bEnabled);

    // After attacking, set back the time to normal
    if (!bEnabled)
        if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(TargetCombat.Get()))
        {
            AttackIndicator->SetVisibility(false);
            PlayerCharacter->HideTip();

            UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
        }
}

void ACombatCharacter::AttachWeapon()
{
    bEquipWeapon = !bEquipWeapon;
    
    Super::AttachWeapon();
}

void ACombatCharacter::SetLockOn(AOWCharacter* Target)
{
    Super::SetLockOn(Target);

    if (!Target) return;
    EnemyController->MoveToLocation(Target->GetActorLocation(), 250.f, false);
}
