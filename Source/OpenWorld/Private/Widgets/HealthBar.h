// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

class UProgressBar;

UCLASS()
class UHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	// ===== Interfaces ========== //

	void UpdateHealth(float Percentage);
	void SetHealthColor(const FLinearColor& LinearColor);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
};
