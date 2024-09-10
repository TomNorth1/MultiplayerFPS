// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#define LOCTEXT_NAMESPACE "Inventory"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	SetIsReplicated(true);
	
	
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey)) 
	{
		for (auto& Item: Items) 
		{
			if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey)) 
			{
				bWroteSomething = Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}
	return bWroteSomething;
}

UItem* UInventoryComponent::AddItem(UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority()) 
	{
		UItem* NewItem = NewObject<UItem>(GetOwner(), Item->GetClass());
		NewItem->SetQuantity(Item->GetQuantity());
		NewItem->OwningInventory = this;
		NewItem->AddedToInventory(this);
		Items.Add(NewItem);
		NewItem->MarkDirtyForReplication();

		return NewItem;
	}
	return nullptr;
}

bool UInventoryComponent::RemoveItem(UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority()) 
	{
		if (Item) 
		{
			Items.RemoveSingle(Item);

			ReplicatedItemsKey++;
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::HasItem(TSubclassOf<class UItem> ItemClass, int32 Qauntity) const
{
	if (UItem* ItemToFind = FindItemByClass(ItemClass)) 
	{
		return ItemToFind->GetQuantity() >= Qauntity;
	}
	return false;
}

UItem* UInventoryComponent::FindItem(UItem* Item) const
{
	if (Item) 
	{
		for (auto& InvItem : Items) 
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass()) 
			{
				return InvItem;
			}
		}
	}
	return nullptr;
}

UItem* UInventoryComponent::FindItemByClass(TSubclassOf<class UItem> ItemClass) const
{
	for(auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass) 
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<class UItem> ItemClass) const
{
	TArray<UItem*> ItemsOfClass;

	for (auto& InvItem : Items) 
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass)) 
		{
			ItemsOfClass.Add(InvItem);
		}
	}

	return ItemsOfClass;
}

void UInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}


void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();
}

FItemAddResult UInventoryComponent::TryAddItem(UItem* Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Qauntity)
{
	UItem* Item = NewObject<UItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Qauntity);
	return TryAddItem_Internal(Item);
}

int32 UInventoryComponent::ConsumeItem(UItem* Item)
{
	if (Item) 
	{
		ConsumeItem(Item, Item->GetQuantity());
	}
	return 0;
}

int32 UInventoryComponent::ConsumeItem(UItem* Item, const int32 Qauntity)
{
	if (GetOwner() && GetOwner()->HasAuthority() && Item) 
	{
		const int32 RemoveQuantity = FMath::Min(Qauntity, Item->GetQuantity());

		ensure(!(Item->GetQuantity() - RemoveQuantity < 0));

		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);
		if (Item->GetQuantity() <= 0) 
		{
			RemoveItem(Item);
		}
		else 
		{
			ClientRefreshInventory();
		}
		return RemoveQuantity;
	}
	return 0;
}


FItemAddResult UInventoryComponent::TryAddItem_Internal(UItem* Item)
{

	if (GetOwner() && GetOwner()->HasAuthority()) 
	{
		const int32 AddAmount = Item->GetQuantity();

		if (Items.Num() + 1 > GetCapacity()) 
		{
			if (Item->bIsStackable)
			{
				if (UItem* ExistingItem = FindItem(Item)) 
				{
					if (ExistingItem->GetQuantity() < Item->MaxStackSize) 
					{
						const int32 CapacityMaxAddAmount = ExistingItem->MaxStackSize - ExistingItem->GetQuantity();
						int32 ActualAddAmount = FMath::Min(AddAmount, CapacityMaxAddAmount);

						FText ErrorText = LOCTEXT("InventoryErrorText", "Couldn't add all of the item to your inventory.");
						if (ActualAddAmount <= 0)
						{
							return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryErrorText", "Couldn't add item to inventory."));
						}
						ExistingItem->SetQuantity(ExistingItem->GetQuantity() + ActualAddAmount);

						ensure(ExistingItem->GetQuantity() <= ExistingItem->MaxStackSize);

						if (ActualAddAmount < AddAmount)
						{
							return FItemAddResult::AddedSome(AddAmount, ActualAddAmount, ErrorText);
						}
						else
						{
							return FItemAddResult::AddedAll(AddAmount);
						}
					}

				}

			}
			return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryCapacityFullText", "Couldn't add item to inventory. Inventory is full!"));
		}

		if (Item->bIsStackable) 
		{
			ensure(Item->GetQuantity() <= Item->MaxStackSize);
			if (UItem* ExistingItem = FindItem(Item)) 
			{
				if (ExistingItem->GetQuantity() < ExistingItem->MaxStackSize) 
				{
					const int32 CapacityMaxAddAmount = ExistingItem->MaxStackSize - ExistingItem->GetQuantity();
					int32 ActualAddAmount = FMath::Min(AddAmount, CapacityMaxAddAmount);

					FText ErrorText = LOCTEXT("InventoryErrorText", "Couldn't add all of the item to your inventory.");
					if (ActualAddAmount <= 0) 
					{
						if (Items.Num() + 1 > GetCapacity()) 
						{
							AddItem(Item);
							return FItemAddResult::AddedAll(AddAmount);
						}
						return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryErrorText", "Couldn't add item to inventory."));
					}
					ExistingItem->SetQuantity(ExistingItem->GetQuantity() + ActualAddAmount);

					ensure(ExistingItem->GetQuantity() <= ExistingItem->MaxStackSize);

					if (ActualAddAmount < AddAmount) 
					{
						return FItemAddResult::AddedSome(AddAmount, ActualAddAmount, ErrorText);
					}
					else 
					{
						return FItemAddResult::AddedAll(AddAmount);
					}
				}
				else 
				{
					if (Items.Num() + 1 > GetCapacity())
					{
						AddItem(Item);
						return FItemAddResult::AddedAll(AddAmount);
					}
					return FItemAddResult::AddedNone(AddAmount, FText::Format(LOCTEXT("InventoryFullStackText", "Couldn't add {ItemName}. You Already have a full stackof this item."),Item->ItemDisplayName));
				}
			}
			else 
			{
				AddItem(Item);

				return FItemAddResult::AddedAll(AddAmount);
			}
		}
		else
		{
			ensure(Item->GetQuantity() == 1);

			AddItem(Item);
			return FItemAddResult::AddedAll(AddAmount);
		}
	}
	check(false);
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}

void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}

#undef LOCTEXT_NAMESPACE