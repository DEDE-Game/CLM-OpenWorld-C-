// Fill out your copyright notice in the Description page of Project Settings.

#include "OWGameMode.h"

AOWGameMode::AOWGameMode()
{
    DefaultInitializer();

}

// ==================== Initializer ==================== //

void AOWGameMode::DefaultInitializer()
{
    // Pawn Class
    static ConstructorHelpers::FClassFinder<APawn> Pawn(
        TEXT("/Game/Game/Blueprints/Characters/BP_GameCharacter")
    );
    DefaultPawnClass = Pawn.Class;

    // Player Controller Class
    static ConstructorHelpers::FClassFinder<APlayerController> Controller(
        TEXT("/Script/OpenWorld.OWPlayerController")
    );
    PlayerControllerClass = Controller.Class;
}
