// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "TheHouse2/Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "Item"

void UItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItem, Quantity);
}

bool UItem::IsSupportedForNetworking() const
{
	return true;
}

void UItem::OnRep_Quantity()
{
	OnItemModified.Broadcast();
}

void UItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, bIsStackable ? MaxStackSize : 1);
		MarkDirtyForReplication();
	}
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::Use(AHorrorCharacter* Character)
{

}

void UItem::AddedToInventory(UInventoryComponent* Inventory)
{

}

void UItem::MarkDirtyForReplication()
{
	++RepKey;


	if (OwningInventory) 
	{
		++OwningInventory->ReplicatedItemsKey;
	}

}


#if WITH_EDITOR
void UItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, Quantity)) 
	{
		Quantity = FMath::Clamp(Quantity, 1, bIsStackable ? MaxStackSize: 1);
	}
}
#endif

UItem::UItem()
{
	ItemDisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	bIsStackable = true;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0;
}

#undef LOCTEXT_NAMESPACE