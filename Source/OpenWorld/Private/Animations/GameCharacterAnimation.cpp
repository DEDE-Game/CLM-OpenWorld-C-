// Fill out your copyright notice in the Description page of Project Settings.

#include "Animations/GameCharacterAnimation.h"
#include "Characters/GameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// ==================== Lifecycle ==================== //

void UGameCharacterAnimation::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    InitializeReferences();    
}

void UGameCharacterAnimation::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    CheckMovements();
}

// ==================== References ==================== //

void UGameCharacterAnimation::InitializeReferences()
{
    GameCharacter = Cast<AGameCharacter>(TryGetPawnOwner());

    if (GameCharacter.IsValid()) CharacterMovement = GameCharacter->GetCharacterMovement();
}

// ==================== Movements ==================== //

void UGameCharacterAnimation::CheckMovements()
{
    if (!CharacterMovement.IsValid()) return;

    FVector Velocity      = CharacterMovement->Velocity;
    FVector ForwardVector = GameCharacter->GetActorForwardVector();
    FVector RightVector   = GameCharacter->GetActorRightVector(); 

    SpeedForward = FVector::DotProduct(Velocity, ForwardVector);  
    SpeedSide    = FVector::DotProduct(Velocity, RightVector);
}
