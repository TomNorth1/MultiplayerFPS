// Fill out your copyright notice in the Description page of Project Settings.


#include "LootableContainer.h"
#include "Components/StaticMeshComponent.h"
#include "TheHouse2/Components/InventoryComponent.h"
#include "TheHouse2/Components/InteractionComponent.h"
#include "Engine/DataTable.h"
#include "TheHouse2/World/ItemSpawn.h"
#include "TheHouse2/Items/Item.h"
#include "TheHouse2/Player/HorrorCharacter.h"
// Sets default values

#define LOCTEXT_NAMESPACE "LootableActor"
ALootableContainer::ALootableContainer()
{
	LootContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>("LootContainerMesh");
	SetRootComponent(LootContainerMesh);

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("LootInteractable");
	InteractionComponent->InteractableActionText = LOCTEXT("LootActorText", "Loot");
	InteractionComponent->InteractableNameText = LOCTEXT("LootActorName", "Chest");
	InteractionComponent->SetupAttachment(GetRootComponent());

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	InventoryComponent->SetCapacity(20);

	LootRolls = FIntPoint(2, 8);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ALootableContainer::BeginPlay()
{
	Super::BeginPlay();

	InteractionComponent->OnInteract.AddDynamic(this, &ALootableContainer::OnInteract);

	if (HasAuthority() && LootTable) 
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		int32 Rolls = FMath::RandRange(LootRolls.GetMin(), LootRolls.GetMax());

		for (int32 i = 0; i < Rolls; ++i) 
		{
			const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

			ensure(LootRow);

			float ProbabilityRoll = FMath::RandRange(0.f, 1.f);

			while (ProbabilityRoll > LootRow->Probability) 
			{
				LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
				ProbabilityRoll = FMath::RandRange(0.f, 1.f);
			}

			if (LootRow && LootRow->Items.Num()) 
			{
				for (auto& ItemClass : LootRow->Items) 
				{
					if (ItemClass) 
					{
						const int32 Quantity = Cast<UItem>(ItemClass->GetDefaultObject())->GetQuantity();
						InventoryComponent->TryAddItemFromClass(ItemClass, Quantity);
					}
				}
			}
		}
	}

}

void ALootableContainer::OnInteract(class AHorrorCharacter* Character)
{
	if (Character) 
	{
		Character->SetLootSource(InventoryComponent);
	}
}


#undef LOCTEXT_NAMESPACE