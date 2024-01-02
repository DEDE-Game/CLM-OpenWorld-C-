// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/GameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"

AGameCharacter::AGameCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Pawn
	bUseControllerRotationYaw = bUseControllerRotationPitch = bUseControllerRotationRoll = false;

	// Capsule
	GetCapsuleComponent()->SetCapsuleHalfHeight(103.5f);
	GetCapsuleComponent()->SetCapsuleRadius(21.5f);

	// Movement
	/* General */
	GetCharacterMovement()->MaxAcceleration = 500.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	/* Walking */
	GetCharacterMovement()->GroundFriction = 1.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 100.f;

	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// ...
	DefaultInitializer();
}

void AGameCharacter::DefaultInitializer()
{
	ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Look.IA_Look'")
	);
	LookAction = LookActionAsset.Object;

	ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Move.IA_Move'")
	);
	MoveAction = MoveActionAsset.Object;

	ConstructorHelpers::FObjectFinder<UInputAction> SprintActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Sprint.IA_Sprint'")
	);
	SprintAction = SprintActionAsset.Object;

	ConstructorHelpers::FObjectFinder<UInputAction> WalkActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Walk.IA_Walk'")
	);
	WalkAction = WalkActionAsset.Object;

	ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Game/Inputs/IA_Jump.IA_Jump'")
	);
	JumpAction = JumpActionAsset.Object;
}

// ==================== Lifecycles ==================== //

void AGameCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGameCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent *EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(LookAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInput->BindAction(MoveAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Move);

		EnhancedInput->BindAction(SprintAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Sprint);
		EnhancedInput->BindAction(SprintAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Sprint);

		EnhancedInput->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::Walk);
		EnhancedInput->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ThisClass::Walk);
		
		EnhancedInput->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &ThisClass::DoJump);
	}
}

void AGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ==================== Locomotions ==================== //

void AGameCharacter::Look(const FInputActionValue &InputValue)
{
	const FVector2D Value = InputValue.Get<FVector2D>();

	AddControllerYawInput(Value.X);
	AddControllerPitchInput(Value.Y);
}

void AGameCharacter::Move(const FInputActionValue &InputValue)
{
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

void AGameCharacter::Sprint(const FInputActionValue &InputValue)
{
	const bool Value = InputValue.Get<bool>();

	GetCharacterMovement()->MaxWalkSpeed = Value ? SprintSpeed : RunSpeed;
}

void AGameCharacter::Walk(const FInputActionValue &InputValue)
{
	const bool Value = InputValue.Get<bool>();

	GetCharacterMovement()->MaxWalkSpeed = Value ? WalkSpeed : RunSpeed;
}

void AGameCharacter::DoJump()
{
	Jump();

	if (Montages.Contains("Jump"))
		PlayAnimMontage(Montages["Jump"].LoadSynchronous());
}

void AGameCharacter::Landed(const FHitResult &Hit)
{
	Super::Landed(Hit);

	if (Montages.Contains("Jump"))
		PlayAnimMontage(Montages["Jump"].LoadSynchronous(), 1.f, TEXT("Land"));
}

// ==================== Audio ==================== //

void AGameCharacter::PlayFootstepSound()
{
	if (FootstepSounds.Contains(TEXT("Concrete")))
		UGameplayStatics::PlaySoundAtLocation(this, FootstepSounds["Concrete"].LoadSynchronous(), GetActorLocation());
}
