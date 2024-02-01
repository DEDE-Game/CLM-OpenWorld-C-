// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OWPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class OPENWORLD_API AOWPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AOWPlayerController();

	// ===== Input ========== //

	void UseGameInput();

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void DefaultInitializer();

	// ===== Input ========== //

	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputMappingContext> GameInput;
};
