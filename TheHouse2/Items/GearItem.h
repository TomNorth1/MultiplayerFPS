// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TheHouse2/Items/EquipableItem.h"
#include "GearItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class THEHOUSE2_API UGearItem : public UEquipableItem
{
	GENERATED_BODY()
public:
	UGearItem();

	virtual bool Equip(class AHorrorCharacter* Character) override;
	virtual bool UnEquip(class AHorrorCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gear")
	class UStaticMesh* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gear", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DamageDefenceMultiplier;
};
