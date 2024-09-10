// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthItem.h"


#define LOCTEXT_NAMESPACE "Healt Item"

UHealthItem::UHealthItem() 
{
	HealAmmount = 5.0f;
	UseActionText = LOCTEXT("ItemUseActionText", "Use");

}

void UHealthItem::Use(AHorrorCharacter* Character)
{
	UE_LOG(LogTemp, Warning, TEXT("Used Item"));
}
#undef LOCTEXT_NAMESPACE