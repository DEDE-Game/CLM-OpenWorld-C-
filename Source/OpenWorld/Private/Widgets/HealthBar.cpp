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
    HealthBar->WidgetStyle.BackgroundImage.OutlineSettings.Color = LinearColor;
}
