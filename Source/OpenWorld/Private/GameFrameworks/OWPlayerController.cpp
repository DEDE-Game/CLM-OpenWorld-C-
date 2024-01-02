// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFrameworks/OWPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AOWPlayerController::AOWPlayerController()
{
    DefaultClassInitializer();
}

// ==================== Initializer ==================== //

void AOWPlayerController::DefaultClassInitializer()
{
    // Game Input Mapping Context
    ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContext(
        TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Game/Inputs/IMC_GameInput.IMC_GameInput'")
    );

    GameInput = MappingContext.Object;
}

// ==================== Lifecycles ==================== //

void AOWPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UseGameInput();
}

// ==================== Input ==================== //

void AOWPlayerController::UseGameInput()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(GameInput.LoadSynchronous(), 0);
    }
}

