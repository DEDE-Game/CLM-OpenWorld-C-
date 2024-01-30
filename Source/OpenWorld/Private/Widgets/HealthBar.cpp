// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/HealthBar.h"
#include "Components/ProgressBar.h"

void UHealthBar::UpdateHealth(float Percentage)
{
    HealthBar->SetPercent(Percentage);
}

void UHealthBar::SetHealthColor(const FLinearColor& LinearColor)
{
    HealthBar->SetFillColorAndOpacity(LinearColor);

    // Set the outline settings
    FProgressBarStyle CurrentStyle = HealthBar->GetWidgetStyle();
    FProgressBarStyle NewStyle     = CurrentStyle;
    NewStyle.BackgroundImage.OutlineSettings.Color = FSlateColor(LinearColor);
    HealthBar->SetWidgetStyle(NewStyle);
}
