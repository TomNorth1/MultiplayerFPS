// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponItem.h"
#include "TheHouse2/Player/HorrorCharacter.h"

UWeaponItem::UWeaponItem()
{
    
}

bool UWeaponItem::Equip(AHorrorCharacter* Character)
{
    bool bEquipSuccessful = Super::Equip(Character);

    if(bEquipSuccessful && Character)
    {
        Character->EquipWeapon(this);
    }
    return bEquipSuccessful;
}


bool UWeaponItem::UnEquip(AHorrorCharacter* Character)
{
    bool bUnEquipSuccessful = Super::UnEquip(Character);
    if(bUnEquipSuccessful && Character)
    {
        Character->UnEquipWeapon();
    }
    return bUnEquipSuccessful;
}
