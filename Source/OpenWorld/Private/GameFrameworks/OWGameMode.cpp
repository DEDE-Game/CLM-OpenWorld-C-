// Fill out your copyright notice in the Description page of Project Settings.

#include "OWGameMode.h"
#include "Controllers/DaytimeController.h"
#include "Controllers/WeatherController.h"

AOWGameMode::AOWGameMode()
{
    DefaultClassInitializer();

    // Controllers
    DaytimeController = CreateDefaultSubobject<UDaytimeController>(TEXT("Daytime Controller"));
    WeatherController = CreateDefaultSubobject<UWeatherController>(TEXT("Weather Controller"));
}

// ==================== Initializer ==================== //

void AOWGameMode::DefaultClassInitializer()
{
    // Pawn Class
    ConstructorHelpers::FClassFinder<APawn> Pawn(
        TEXT("/Game/Game/Blueprints/Characters/BP_GameCharacter")
    );
    DefaultPawnClass = Pawn.Class;

    // Player Controller Class
    ConstructorHelpers::FClassFinder<APlayerController> Controller(
        TEXT("/Script/OpenWorld.OWPlayerController")
    );
    PlayerControllerClass = Controller.Class;
}
