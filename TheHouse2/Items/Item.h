// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

UENUM(BlueprintType)
enum class EItemRarity : uint8 
{
    IR_TierOne UMETA(DisplayName= "Tier One"),
    IR_TierTwo UMETA(DisplayName = "Tier Two"),
    IR_TierThree UMETA(DisplayName = "Tier Three"),
    IR_TierFour UMETA(DisplayName = "Tier Four")
};


UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class THEHOUSE2_API UItem : public UObject
{
	GENERATED_BODY()

protected:
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool IsSupportedForNetworking() const override;


#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
    UItem();


    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    class UStaticMesh* PickupMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    class UTexture2D* Thumbnail;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    FText ItemDisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
    FText ItemDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    FText UseActionText;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    EItemRarity Rarity;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    bool bIsStackable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 2, EditCondition = bIsStackable))
    int32 MaxStackSize;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
    TSubclassOf<class UItemTooltip> ItemTooltip;

    UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition = bIsStackable))
    int32 Quantity;

    UPROPERTY()
    class UInventoryComponent* OwningInventory;

    UPROPERTY()
    int32 RepKey;

    UPROPERTY(BlueprintAssignable)
    FOnItemModified OnItemModified;

    UFUNCTION()
    void OnRep_Quantity();

    UFUNCTION(BlueprintCallable, Category = "Item")
    void SetQuantity(const int32 NewQuantity);

    UFUNCTION(BlueprintPure, Category = "Item")
    FORCEINLINE int32 GetQuantity() const { return Quantity; }

    UFUNCTION(BlueprintPure, Category = "Item")
    virtual bool ShouldShowInInventory() const;

    virtual void Use(class AHorrorCharacter* Character);
    virtual void AddedToInventory(class UInventoryComponent* Inventory);

    void MarkDirtyForReplication();

};
