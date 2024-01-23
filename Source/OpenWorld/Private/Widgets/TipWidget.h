// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TipWidget.generated.h"

class UTextBlock;

UCLASS()
class UTipWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void ShowTip(const FString& Text);
	FORCEINLINE void HideTip()
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TipText;
};
