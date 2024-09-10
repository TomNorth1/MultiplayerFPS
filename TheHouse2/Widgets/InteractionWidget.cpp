// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionWidget.h"
#include "TheHouse2/Components/InteractionComponent.h"

void UInteractionWidget::UpdateInteractionWidget(UInteractionComponent* InteractionComponent)
{
	OwningInteractionComponent = InteractionComponent;
	OnUpdateInteractionWidget();
}


