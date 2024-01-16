// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapons/Weapon.h"

APlayerCharacter::APlayerCharacter()
{
	// Movement
	/* General */
	GetCharacterMovement()->MaxAcceleration = 500.f;

	/* Walking */
	GetCharacterMovement()->GroundFriction = 1.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 100.f;
	/* Crouching */
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag 	   = true;
	SpringArm->CameraLagSpeed = 15.f;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

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

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ReferencesInitializer();

	// Default value for last location
	FVector CurrentLocation = GetActorLocation();
	CurrentLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	LastLocation1 = LastLocation2 = CurrentLocation;

	// !TESTING!
	CarriedWeapon = GetWorld()->SpawnActor<AWeapon>(GivenWeapon);
	CarriedWeapon->EquipTo(this, TEXT("Back Socket"));
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent *EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(LookAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInput->BindAction(MoveAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Move);

		EnhancedInput->BindAction(SprintAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::Sprint);
		EnhancedInput->BindAction(SprintAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Sprint);

		EnhancedInput->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::Walk);
		EnhancedInput->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Walk);
		
		EnhancedInput->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::DoJump);

		EnhancedInput->BindAction(CrouchAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::DoCrouch);
		EnhancedInput->BindAction(CrouchAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::DoCrouch);

		EnhancedInput->BindAction(SwapWeaponAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::ChangeWeapon);

		EnhancedInput->BindAction(AttackAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Attack);

		EnhancedInput->BindAction(BlockAction.LoadSynchronous(), ETriggerEvent::Started, this, &ThisClass::Block);
		EnhancedInput->BindAction(BlockAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Block);
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

	const FVector2D Value = InputValue.Get<FVector2D>();

	// Getting Controller's Yaw rotation only
	FRotator Rotation = GetController()->GetControlRotation();
	Rotation.Roll = Rotation.Pitch = 0.f;

	// Moving Y Axis depends on the character rotation
	const FVector ForwardDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, Value.Y);

	// Moving X Axis depends on the character rotation
	const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, Value.X);
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
}

void APlayerCharacter::ChangeWeapon(const FInputActionValue& InputValue)
{
	float Value = InputValue.Get<float>();

	SwapWeapon(Value);
}

void APlayerCharacter::LockNearest()
{
	if (!bEquipWeapon) return;

	// Find nearest using sphere trace
	FVector    TraceLocation = GetActorLocation();
	FHitResult TraceResult;

	// Only trace for pawn
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	UKismetSystemLibrary::SphereTraceSingleForObjects(
		this,
		TraceLocation,
		TraceLocation,
		500.f,
		ObjectTypes,
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		TraceResult,
		true
	);

	if (!TraceResult.bBlockingHit)
	{
		SetLockOn(nullptr);
		
		return;
	}

	if (AOWCharacter* Other = Cast<AOWCharacter>(TraceResult.GetActor()); Other && IsEnemy(Other))
		SetLockOn(Other);
	else
		SetLockOn(nullptr);
}

void APlayerCharacter::Attack()
{
	LockNearest();

	Super::Attack();
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
