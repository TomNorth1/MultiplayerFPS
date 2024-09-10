// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TheHouse2/Items/Item.h"
#include "HealthItem.generated.h"

/**
 * 
 */
UCLASS()
class THEHOUSE2_API UHealthItem : public UItem
{
	GENERATED_BODY()
	
public:
	UHealthItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Healing")
	float HealAmmount;
	
	virtual void Use(class AHorrorCharacter* Character) override;
};
