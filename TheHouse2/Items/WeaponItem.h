// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TheHouse2//Items/EquipableItem.h"
#include "WeaponItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class THEHOUSE2_API UWeaponItem : public UEquipableItem
{
	GENERATED_BODY()
public:
	UWeaponItem();

	virtual bool Equip(class AHorrorCharacter* Character) override;
	virtual bool UnEquip(class AHorrorCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> WeaponClass;
};
