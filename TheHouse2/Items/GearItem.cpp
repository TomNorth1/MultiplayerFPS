// Fill out your copyright notice in the Description page of Project Settings.


#include "GearItem.h"
#include "TheHouse2/Player/HorrorCharacter.h"

UGearItem::UGearItem() 
{
	DamageDefenceMultiplier = 0.1f;

}

bool UGearItem::Equip(AHorrorCharacter* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character) 
	{
		Character->EquipGear(this);
	}
	return bEquipSuccessful;
}

bool UGearItem::UnEquip(AHorrorCharacter* Character)
{
	bool bUnEquipSuccessful = Super::UnEquip(Character);

	if (bUnEquipSuccessful && Character) 
	{
		Character->UnEquipGear(this);
	}
	return bUnEquipSuccessful;
}
