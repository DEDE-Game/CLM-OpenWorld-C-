// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/OWCharacter.h"
#include "Enums/CollisionChannel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Weapons/Weapon.h"

AOWCharacter::AOWCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Pawn
	bUseControllerRotationYaw = bUseControllerRotationPitch = bUseControllerRotationRoll = false;

	// Make the mesh world dynamic so it can be hit by weapon
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// Movement
	/* General */
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxAcceleration = 500.f;

	/* Walking */
	GetCharacterMovement()->GroundFriction = 1.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 100.f;
	/* Crouching */
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// ...
	DefaultInitializer();
}

void AOWCharacter::DefaultInitializer()
{
	static ConstructorHelpers::FObjectFinder<USoundBase> HitfleshAsset(
		TEXT("/Script/MetasoundEngine.MetaSoundSource'/Game/Game/Audio/Combats/MS_HitFlesh.MS_HitFlesh'")
	);
	HitfleshSound = HitfleshAsset.Object;

	static ConstructorHelpers::FObjectFinder<USoundBase> BlockingAsset(
		TEXT("/Script/MetasoundEngine.MetaSoundSource'/Game/Game/Audio/Combats/MS_Blocking.MS_Blocking'")
	);
	BlockingSound = BlockingAsset.Object;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BloodSplashAsset(
		TEXT("/Script/Niagara.NiagaraSystem'/Game/Game/VFX/Combat/NS_BloodSplash.NS_BloodSplash'")
	);
	BloodSplash = BloodSplashAsset.Object;
}

// ==================== Lifecycles ==================== //

void AOWCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AOWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LockOn();
}

// ==================== Locomotions ==================== //

void AOWCharacter::ToggleWalk(bool bToggled)
{
	bool bTargeting = TargetCombat.IsValid();

	// When the character is still locking on enemy target, make it still using walk speed
	GetCharacterMovement()->MaxWalkSpeed = bToggled   ? WalkSpeed : 
										   bTargeting ? WalkSpeed : RunSpeed;
}

void AOWCharacter::ToggleSprint(bool bToggled)
{
	// Determine the target value
	float ToggledSpeed = TargetCombat.IsValid() ? RunSpeed  : SprintSpeed;
	float NormalSpeed  = TargetCombat.IsValid() ? WalkSpeed : RunSpeed;

	GetCharacterMovement()->MaxWalkSpeed = bToggled ? ToggledSpeed : NormalSpeed;
}

void AOWCharacter::ToggleCrouch(bool bToggled)
{
	bool bCrouched = GetCharacterMovement()->IsCrouching();

	if (bCrouched) UnCrouch();
	else			 Crouch();
}

void AOWCharacter::MoveForward()
{
	GetCharacterMovement()->AddImpulse(GetActorForwardVector() * 200.f, true);
}

// ==================== Combat ==================== //

const bool AOWCharacter::IsEnemy(AOWCharacter* Other) const 
{
	if (GetTeam() == ETeam::T_Neutral || Other->GetTeam() == ETeam::T_Neutral) return false;

	return GetTeam() != Other->GetTeam();
}

void AOWCharacter::ToggleBlock(bool bToggled)
{
	if (!Montages.Contains("Blocking")) return;

	if (bToggled) PlayAnimMontage(Montages["Blocking"].LoadSynchronous());
	else		  StopAnimMontage(Montages["Blocking"].LoadSynchronous());

	// Reset combat
	ToggleMovement(true);
	EnableWeapon(false);
}

void AOWCharacter::EnableWeapon(bool bEnabled)
{
	if (!CarriedWeapon.IsValid()) return;

	CarriedWeapon->EnableCollision(bEnabled);
}

void AOWCharacter::AttachWeapon()
{
	FName SocketName  = bEquipWeapon ? TEXT("Hand Socket") : TEXT("Back Socket");

	CarriedWeapon->EquipTo(this, SocketName);
}

void AOWCharacter::SwapWeapon(float Value)
{
	if (!CarriedWeapon.IsValid() || !Montages.Contains("Equipping")) return;

	// Determine which section to play the montage
	FName SectionName = bEquipWeapon ? TEXT("Unequip") : TEXT("Equip");
	bEquipWeapon 	  = !bEquipWeapon;

	// Play montage to trigger attach weapon
	PlayAnimMontage(Montages["Equipping"].LoadSynchronous(), 1.f);
}

void AOWCharacter::SetLockOn(AOWCharacter* Target)
{
	TargetCombat = Target;

	// Adjust orient movement to false to make the locking works
	GetCharacterMovement()->bOrientRotationToMovement = !TargetCombat.IsValid();
	ToggleWalk(TargetCombat.IsValid());
}

void AOWCharacter::LockOn()
{
	if (!TargetCombat.IsValid()) return;

	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation 	 = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetCombat->GetActorLocation());
	
	// Only takes the new Yaw
	NewRotation.Pitch = CurrentRotation.Pitch;
	NewRotation.Roll  = CurrentRotation.Roll;

	SetActorRotation(NewRotation);
}

void AOWCharacter::HitReaction(const FVector& ImpactPoint)
{
	if (!Montages.Contains("Blocking") || !Montages.Contains("Hit React")) return;

	// Get datas
	FVector CurrentLocation = GetActorLocation();
	FVector Forward         = GetActorForwardVector();
	FVector HitDirection    = (ImpactPoint - CurrentLocation).GetSafeNormal2D();

	// Depends on rad angle...its 0: Front; 1: Left; 2: Right; 3: Back
	float DotProduct = FVector::DotProduct(Forward, HitDirection);
	int32 RadAngle   = FMath::FloorToInt32(FMath::Acos(DotProduct));

	// Check if the player succeed block the hit
	bSucceedBlocking = GetCurrentMontage() == Montages["Blocking"].Get() && RadAngle == 0;

	// Animation Montage (Depends on succeed blocking or no)
	FName MontageToPlay  = bSucceedBlocking ? TEXT("Blocking") : TEXT("Hit React");
	FName MontageSection = bSucceedBlocking ? TEXT("Blocking") : *FString::Printf(TEXT("From%d"), RadAngle);
	PlayAnimMontage(Montages[MontageToPlay].LoadSynchronous(), 1.f, MontageSection);

	// Knock back
	float KnockbackPower = bSucceedBlocking ? 200.f : 400.f;
	GetCharacterMovement()->AddImpulse(-HitDirection * KnockbackPower, true);
}

void AOWCharacter::ComboOver()
{
	AttackCount = 0;
}

void AOWCharacter::Attack()
{
	if (!Montages.Contains("Attacking") || !bEquipWeapon || !bCanMove) return;

	bCanMove = false;
	GetCharacterMovement()->StopMovementImmediately();

	// Play montage
	FName AttackCombo     = *FString::Printf(TEXT("%d"), AttackCount);
	UAnimMontage* Montage = Montages["Attacking"].LoadSynchronous(); 
	PlayAnimMontage(Montage, 1.f, AttackCombo);

	// Updating combo, don't forget to update the combo over too
	AttackCount = (AttackCount + 1) % 3;
	GetWorldTimerManager().SetTimer(
		ComboOverHandler,
		this,
		&ThisClass::ComboOver,
		ComboOverTimer
	);
}

void AOWCharacter::OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& ImpactPoint)
{
	if (!Montages.Contains("Hit React")) return;

	// Hit React
	HitReaction(ImpactPoint);

	// Reset combat
	ToggleMovement(true);
	EnableWeapon(false);

	// Show hit visualization
	if (!bSucceedBlocking) UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, BloodSplash.LoadSynchronous(), ImpactPoint);

	// Sound (Blocking or hitflesh sound)
	UGameplayStatics::PlaySoundAtLocation(
		this, 
		bSucceedBlocking ? BlockingSound.LoadSynchronous() : HitfleshSound.LoadSynchronous(), 
		ImpactPoint
	);
}

// ==================== Audio ==================== //

void AOWCharacter::PlayFootstepSound()
{
	bool bCrouching 		= GetCharacterMovement()->IsCrouching();
	FVector CurrentLocation = GetActorLocation();

	// Only make noises when the player is not crouching ofc
	if (!bCrouching)
		MakeNoise(1.f, this, CurrentLocation, 500.f);

	if (FootstepSounds.Contains(TEXT("Concrete")))
		UGameplayStatics::PlaySoundAtLocation(
			this, 
			FootstepSounds["Concrete"].LoadSynchronous(), 
			CurrentLocation,
			bCrouching ? .4f : 1.f,
			bCrouching ? .8f : 1.f
		);
}
