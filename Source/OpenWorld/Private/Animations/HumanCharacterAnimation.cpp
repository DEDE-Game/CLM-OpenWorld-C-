// Fill out your copyright notice in the Description page of Project Settings.

#include "Animations/HumanCharacterAnimation.h"
#include "Characters/OWCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// ==================== Lifecycle ==================== //

void UHumanCharacterAnimation::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    InitializeReferences();    
}

void UHumanCharacterAnimation::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    UpdateMovements();
    UpdateCombat();
}

// ==================== References ==================== //

void UHumanCharacterAnimation::InitializeReferences()
{
    OWCharacter = Cast<AOWCharacter>(TryGetPawnOwner());

    if (OWCharacter.IsValid()) CharacterMovement = OWCharacter->GetCharacterMovement();
}

// ==================== Movements ==================== //

void UHumanCharacterAnimation::UpdateMovements()
{
    if (!CharacterMovement.IsValid()) return;

    FVector Velocity      = CharacterMovement->Velocity;
    FVector ForwardVector = OWCharacter->GetActorForwardVector();
    FVector RightVector   = OWCharacter->GetActorRightVector(); 

    SpeedForward = FVector::DotProduct(Velocity, ForwardVector);  
    SpeedSide    = FVector::DotProduct(Velocity, RightVector);

    bCrouched    = CharacterMovement->IsCrouching();
}

// ==================== Combats ==================== //

void UHumanCharacterAnimation::UpdateCombat()
{
    if (!OWCharacter.IsValid()) return;

    bEquipWeapon = OWCharacter->IsEquippingWeapon();
}
