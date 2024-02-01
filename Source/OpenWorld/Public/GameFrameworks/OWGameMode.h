// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OWGameMode.generated.h"

UCLASS()
class OPENWORLD_API AOWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOWGameMode();

private:
	// ===== Initializer ========== //

	void DefaultInitializer();

};
