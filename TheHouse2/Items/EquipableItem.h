// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TheHouse2/Items/Item.h"
#include "EquipableItem.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EEquipableSlot : uint8 
{
	EIS_Helmet UMETA(DisplayName = "Helmet"),
	EIS_Armour UMETA(DisplayName = "Armour"),
	EIS_Backpack UMETA(DisplayName = "Backpack"),
	EIS_Weapon UMETA(DisplayName = "Weapon"),
	EIS_Throwable UMETA(DisplayName = "Throwable")
};

UCLASS(Abstract, NotBlueprintable)
class THEHOUSE2_API UEquipableItem : public UItem
{
	GENERATED_BODY()
	
public:
	UEquipableItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipables")
	EEquipableSlot Slot;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Use(class AHorrorCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool Equip(class AHorrorCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool UnEquip(class AHorrorCharacter* Character);

	virtual void AddedToInventory(class UInventoryComponent* Inventory) override;
	
	virtual bool ShouldShowInInventory() const override;

	UFUNCTION(BlueprintPure, Category = "Equippables")
	bool IsEquipped() { return bEquipped; }

	void SetEquipped(bool bNewEquipped);


protected:
	UPROPERTY(ReplicatedUsing = EquipStatusChanged)
	bool bEquipped;

	UFUNCTION()
	void EquipStatusChanged();

};
