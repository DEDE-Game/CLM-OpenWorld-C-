// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/TipWidget.h"
#include "Components/TextBlock.h"

void UTipWidget::ShowTip(const FString& Text)
{
    TipText->SetText(FText::FromString(Text));
    SetVisibility(ESlateVisibility::Visible);
}
