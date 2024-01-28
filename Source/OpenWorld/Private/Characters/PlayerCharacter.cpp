// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFrameworks/OWPlayerController.h"
#include "GameFrameworks/OWHUD.h"
#include "EnhancedInputComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Weapons/MeleeWeapon.h"

APlayerCharacter::APlayerCharacter()
{
	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag 	   = true;
	SpringArm->CameraLagSpeed = 15.f;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Takedown area
	TakedownArea = CreateDefaultSubobject<UBoxComponent>(TEXT("Takedown Area"));
	TakedownArea->SetupAttachment(RootComponent);
	TakedownArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// Inventory
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	// ...
	Team = ETeam::T_Friend;
	DefaultInitializer();
}

void APlayerCharacter::DefaultInitializer()
{
	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Look.IA_Look'")
	);
	LookAction = LookActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Move.IA_Move'")
	);
	MoveAction = MoveActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> SprintActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Sprint.IA_Sprint'")
	);
	SprintAction = SprintActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> WalkActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Walk.IA_Walk'")
	);
	WalkAction = WalkActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Jump.IA_Jump'")
	);
	JumpAction = JumpActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> CrouchActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Crouch.IA_Crouch'")
	);
	CrouchAction = CrouchActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> SwapWeaponActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_SwapWeapon.IA_SwapWeapon'")
	);
	SwapWeaponAction = SwapWeaponActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> AttackActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Attack.IA_Attack'")
	);
	AttackAction = AttackActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> BlockActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Block.IA_Block'")
	);
	BlockAction = BlockActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> DodgeActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Dodge.IA_Dodge'")
	);
	DodgeAction = DodgeActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Interact.IA_Interact'")
	);
	InteractAction = InteractActionAsset.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> GlobalParamAsset(
		TEXT("/Script/Engine.MaterialParameterCollection'/Game/Game/Materials/MP_Global.MP_Global'")
	);
	GlobalMatParam = GlobalParamAsset.Object;
}

void APlayerCharacter::ReferencesInitializer()
{
	GlobalMatParamIns = GetWorld()->GetParameterCollectionInstance(GlobalMatParam.LoadSynchronous());
}

// ==================== Lifecycles ==================== //

void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	if (Inventory) Inventory->RefreshSlot();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ReferencesInitializer();

	// Default value for last location
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	LastLocation1 = LastLocation2 = CurrentLocation;

	// Events
	TakedownArea->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnEnterTakedown);
	TakedownArea->OnComponentEndOverlap  .AddDynamic(this, &ThisClass::OnLeaveTakedown);
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Get the reference
	OWPlayerController = Cast<AOWPlayerController>(NewController);

	if (OWPlayerController.IsValid())
		OWHUD = Cast<AOWHUD>(OWPlayerController->GetHUD());
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent *EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(LookAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Look);

		EnhancedInput->BindAction(MoveAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInput->BindAction(MoveAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Move);

		EnhancedInput->BindAction(SprintAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::Sprint);
		EnhancedInput->BindAction(SprintAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Sprint);

		EnhancedInput->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::Walk);
		EnhancedInput->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Walk);
		
		EnhancedInput->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::DoJump);

		EnhancedInput->BindAction(CrouchAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::DoCrouch);
		EnhancedInput->BindAction(CrouchAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::DoCrouch);

		EnhancedInput->BindAction(SwapWeaponAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::ChangeWeapon);

		EnhancedInput->BindAction(AttackAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::ChargeAttack);
		EnhancedInput->BindAction(AttackAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Attack);

		EnhancedInput->BindAction(BlockAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::Block);
		EnhancedInput->BindAction(BlockAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Block);

		EnhancedInput->BindAction(DodgeAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Dodge);

		EnhancedInput->BindAction(InteractAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Interact);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AffectsFoliage();
}

// ==================== Locomotions ==================== //

void APlayerCharacter::Look(const FInputActionValue &InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();

	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}

void APlayerCharacter::Move(const FInputActionValue &InputValue)
{
	if (!bCanMove) return;

	MovementInput = InputValue.Get<FVector2D>();

	// Getting Controller's Yaw rotation only
	FRotator Rotation = GetController()->GetControlRotation();
	Rotation.Roll = Rotation.Pitch = 0.f;

	// Moving Y Axis depends on the character rotation
	const FVector ForwardDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementInput.Y);

	// Moving X Axis depends on the character rotation
	const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementInput.X);
}

void APlayerCharacter::Sprint(const FInputActionValue &InputValue)
{
	const bool Value = InputValue.Get<bool>();

	ToggleSprint(Value);
}

void APlayerCharacter::Walk(const FInputActionValue &InputValue)
{
	const bool Value = InputValue.Get<bool>();

	ToggleWalk(Value);
}

void APlayerCharacter::DoJump()
{
	Jump();

	if (Montages.Contains("Jump"))
		PlayAnimMontage(Montages["Jump"].LoadSynchronous());
}

void APlayerCharacter::DoCrouch(const FInputActionValue& InputValue)
{
	const bool Value = InputValue.Get<bool>();

	ToggleCrouch(Value);
}

void APlayerCharacter::Landed(const FHitResult &Hit)
{
	Super::Landed(Hit);

	if (!Montages.Contains("Jump")) return;

	PlayAnimMontage(Montages["Jump"].LoadSynchronous(), 1.f, TEXT("Land"));
}

// ==================== Combat ==================== //

void APlayerCharacter::Block(const FInputActionValue& InputValue)
{
	if (!bEquipWeapon) return;

	const bool Value = InputValue.Get<bool>();

	ToggleBlock(Value);

	// Make player lock at nearest enemy
	if (Value) LockNearest();
}

void APlayerCharacter::ChangeWeapon(const FInputActionValue& InputValue)
{
	if (GetCurrentMontage()) return;

	float Value = InputValue.Get<float>();

	Inventory->ChangeWeapon(Value - 1);
	SwapWeapon();
}

void APlayerCharacter::Dodge()
{
	// We're only able to dodge in combat and also with WASD key for directional dodge
	if (!TargetCombat.IsValid() || MovementInput.Size() != 1.f) return;

	// Reset Combat
	ToggleMovement(true);
	EnableWeapon(false);

	// Play Montage
	FName SectionName = *FString::Printf(TEXT("%d%d"), FMath::FloorToInt32(MovementInput.X), FMath::FloorToInt32(MovementInput.Y));

	PlayAnimMontage(Montages["Dodging"].LoadSynchronous(), 1.f, SectionName);
}

void APlayerCharacter::ChargeAttack()
{
	if (!bEquipWeapon) return;

	DamageMultiplier += DamageMultiplierRate * GetWorld()->GetDeltaSeconds();

	// Start timer for the first time
	if (!GetWorldTimerManager().IsTimerActive(ChargeTimer) && !bCharging)
		 GetWorldTimerManager().SetTimer(ChargeTimer, this, &ThisClass::DoChargeAttack, ChargeAfter);
}

void APlayerCharacter::DoChargeAttack()
{
	if (!Montages.Contains("Charge Attack")) return;

	LockNearest();
	PlayAnimMontage(Montages["Charge Attack"].LoadSynchronous());

	bCharging = true;
}

void APlayerCharacter::Attack()
{
	if (!CarriedWeapon.IsValid()) return;

	// Takedown
	if (TargetTakedown.IsValid())
		PerformTakedown();
	// Charge Attack
	else if (bCharging)
	{
		bCanMove = false;
		GetCharacterMovement()->StopMovementImmediately();

		CarriedWeapon->SetTempDamage(
			CarriedWeapon->GetDamage() * DamageMultiplier
		);

		FName ChargeAttackSection = *FString::Printf(TEXT("%sChargeAttack"), *CarriedWeapon->GetWeaponName());
		PlayAnimMontage(Montages["Attacking"].LoadSynchronous(), 1.f, ChargeAttackSection);
	}
	// Ordinary attack
	else 
	{
		LockNearest();

		Super::Attack();
	}

	// Reset
	DamageMultiplier = 1.f;
	bCharging        = false;
	GetWorldTimerManager().ClearTimer(ChargeTimer);
}

void APlayerCharacter::DeactivateAction()
{
	// Disabling takedown
	TakedownArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::OnLostInterest()
{
	// Re-enable the takedown
	TakedownArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// ==================== Takedown ==================== //

void APlayerCharacter::OnEnterTakedown(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AOWCharacter* Other = Cast<AOWCharacter>(OtherActor);

	if (Other && IsEnemy(Other) && OWHUD.IsValid())
	{
		TargetTakedown = Other;
		ShowTip(TEXT("[LMB] - Perform Takedown"));
	}
}

void APlayerCharacter::OnLeaveTakedown(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TargetTakedown = nullptr;

	if (OWHUD.IsValid()) HideTip();
}

void APlayerCharacter::PerformTakedown()
{
	LockNearest();
	PlayAnimMontage(Montages["Takedown"].LoadSynchronous());

	// Insta kill
	CarriedWeapon->SetTempDamage(1000.f);
}

// ==================== Environments ==================== //

void APlayerCharacter::AffectsFoliage()
{
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	LastLocation1 = FMath::Lerp(LastLocation1, CurrentLocation, .05f);
	LastLocation2 = FMath::Lerp(LastLocation2, CurrentLocation, .03f);

	GlobalMatParamIns->SetVectorParameterValue(TEXT("Player Current Location"), CurrentLocation);
	GlobalMatParamIns->SetVectorParameterValue(TEXT("Player Last Location"), LastLocation1);
	GlobalMatParamIns->SetVectorParameterValue(TEXT("Player Last Location2"), LastLocation2);
}

void APlayerCharacter::Interact()
{
	if (!OverlappingWeapon.IsValid()) return;

	Inventory->AddWeapon(OverlappingWeapon.Get());
}

// ==================== UI ==================== //

void APlayerCharacter::ShowTip(const FString& Text)
{
	if (OWHUD.IsValid())
		OWHUD->ShowTip(Text);
}

void APlayerCharacter::HideTip()
{
	if (OWHUD.IsValid())
		OWHUD->HideTip();
}
