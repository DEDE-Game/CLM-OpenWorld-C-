// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OWHUD.generated.h"

class UTipWidget;

UCLASS()
class OPENWORLD_API AOWHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AOWHUD();

	// ===== Interfaces ========== //

	FORCEINLINE void ShowTip(const FString& Text);
	FORCEINLINE void HideTip();

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

private:
	void DefaultInitializer();

	// ===== Widgets ========== //

	UPROPERTY(EditAnywhere, Category=Widgets)
	TSubclassOf<UUserWidget> TipWidgetClass;

	UPROPERTY()
	TObjectPtr<UTipWidget> TipWidget;

	void ConstructWidgets();
};
