// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFrameworks/OWHUD.h"
#include "Widgets/TipWidget.h"

AOWHUD::AOWHUD()
{
    DefaultInitializer();
}

void AOWHUD::DefaultInitializer()
{
    static ConstructorHelpers::FClassFinder<UTipWidget> TipWidgetAsset(
        TEXT("/Game/Game/Blueprints/Widgets/HUD/WBP_TipWidget")
    );

    TipWidgetClass = TipWidgetAsset.Class;
}

// ==================== Lifecycles ==================== //

void AOWHUD::BeginPlay()
{
    Super::BeginPlay();

    ConstructWidgets();
}

// ==================== Widgets ==================== //

void AOWHUD::ConstructWidgets()
{
    // TipWidget
    TipWidget = CreateWidget<UTipWidget>(GetOwningPlayerController(), TipWidgetClass, TEXT("Tip Widget"));
    TipWidget->AddToViewport();
    TipWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AOWHUD::ShowTip(const FString& Text)
{
    TipWidget->ShowTip(Text);
}

void AOWHUD::HideTip()
{
    TipWidget->HideTip();
}
