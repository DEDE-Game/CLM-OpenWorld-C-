// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OWGameMode.generated.h"

class UDaytimeController;
class UWeatherController;

UCLASS()
class AOWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOWGameMode();

private:
	// ===== Initializer ========== //

	void DefaultClassInitializer();

	// ===== Controllers ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDaytimeController> DaytimeController;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWeatherController> WeatherController;

};
