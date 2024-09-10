// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipableItem.h"
#include "TheHouse2/Player/HorrorCharacter.h"
#include "Net/UnrealNetwork.h"
#include "TheHouse2/Components/InventoryComponent.h"

#define LOCTEXT_NAMESPACE "EquipableItem"

UEquipableItem::UEquipableItem() 
{
	bIsStackable = false;
	bEquipped = false;
	UseActionText = LOCTEXT("ItemUseActionText", "Equip");
}

void UEquipableItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipableItem, bEquipped);
}

void UEquipableItem::Use(AHorrorCharacter* Character)
{

	if (Character && Character->HasAuthority()) 
	{
		if (Character->GetEquippedItems().Contains(Slot)&& !bEquipped) 
		{
			UEquipableItem* AlreadyEquippedItem = *Character->GetEquippedItems().Find(Slot);
			AlreadyEquippedItem->SetEquipped(false);
		}
		SetEquipped(!IsEquipped());
	}
}

bool UEquipableItem::Equip(AHorrorCharacter* Character)
{
	if (Character) 
	{
		return Character->EquipItem(this);
	}
	return false;
}

bool UEquipableItem::UnEquip(AHorrorCharacter* Character)
{
	if (Character) 
	{
		return Character->UnEquipItem(this);
	}
	return false;
}

void UEquipableItem::AddedToInventory(UInventoryComponent* Inventory) 
{
	if(AHorrorCharacter* Character = Cast<AHorrorCharacter>(Inventory->GetOwner()))
	{
		if(Character && !Character->IsLooting())
		{
			if(!Character->GetEquippedItems().Contains(Slot))
			{
				SetEquipped(true);
			}
		}
	}
}

bool UEquipableItem::ShouldShowInInventory() const
{
	return !bEquipped;
}

void UEquipableItem::SetEquipped(bool bNewEquipped)
{
	bEquipped = bNewEquipped;
	EquipStatusChanged();
	MarkDirtyForReplication();
}

void UEquipableItem::EquipStatusChanged()
{
	if (AHorrorCharacter* Character = Cast<AHorrorCharacter>(GetOuter())) 
	{
		if (bEquipped) 
		{
			Equip(Character);

		}
		else 
		{
			UnEquip(Character);
		}
	}

	OnItemModified.Broadcast();
}
#undef LOCTEXT_NAMESPACE