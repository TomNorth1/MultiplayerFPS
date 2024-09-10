// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UObject/ObjectMacros.h"
#include "HorrorCharacter.generated.h"

USTRUCT()
struct FInteractionData 
{
	GENERATED_BODY()

	FInteractionData() 
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}
	
	UPROPERTY()
	class UInteractionComponent* ViewedInteractionComponent;

	UPROPERTY()
	float LastInteractionCheckTime;

	UPROPERTY()
	bool bInteractHeld;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquipableSlot, Slot, const UEquipableItem*, Item);

UCLASS()
class THEHOUSE2_API AHorrorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHorrorCharacter();
    
	
	TMap<EEquipableSlot, UStaticMeshComponent*> PlayerMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Flashlight)
	class USceneComponent* FlashlightPivot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera)
	class USpringArmComponent* FirstPersonSpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInteractionComponent* PlayerInteractionComponent;

	UPROPERTY(EditDefaultsOnly, Category= "Components")
	class USkeletalMeshComponent* MeshFirstPerson;
	
	UPROPERTY(EditAnywhere, Category = "Components")
	class UStaticMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UStaticMeshComponent* ArmourMesh;

	UPROPERTY(EditAnywhere, Category = "Components")
	class UStaticMeshComponent* BackpackMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* PlayerInventory;
	
	UPROPERTY(EditAnywhere, Replicated, Category= "Flashlight")
	class USpotLightComponent* Flashlight;

	UPROPERTY(BlueprintAssignable, Category = "Items")
	FOnEquippedItemsChanged OnEquipedItemsChanged;

	UFUNCTION(BlueprintPure)
	class UStaticMeshComponent* GetSlotStaticMeshComponent(const EEquipableSlot Slot);


	bool EquipItem(class UEquipableItem* Item);
	bool UnEquipItem(class UEquipableItem* Item);

	void EquipGear(class UGearItem* Gear);
	void UnEquipGear(class UGearItem* Gear);

	void EquipWeapon(class UWeaponItem* WeaponItem);
	void UnEquipWeapon();
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquipableSlot, UEquipableItem*> GetEquippedItems() const { return EquippedItems; }

	UFUNCTION(BlueprintCallable,  Category = "Weapons")
	FORCEINLINE class AWeapon* GetEquippedWeapon() const {return EquippedWeapon;}
	
	float ModifyHealth(const float Delta);

	UFUNCTION()
	void OnRep_Health(float OldHealth);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthModified(const float HealthDelta);

	void StartReload();

protected:
	
	void ToggleFlashlight();

	UFUNCTION(Server, Reliable)
	void ServerToggleFlashlight();
	void ServerToggleFlashlight_Implementation();
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;
	
	UFUNCTION()
	void OnRep_EquippedWeapon();
	
	void StartFire();
	void StopFire();
	
	void BeginMeleeAttack();

	UFUNCTION(Server, Reliable)
	void ServerProccessMeleeHit(const FHitResult& MeleeHit);
	void ServerProccessMeleeHit_Implementation(const FHitResult& MeleeHit);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMeleeFX();
	void MulticastPlayMeleeFX_Implementation();

	UPROPERTY()
	float LastMeleeAttackTime;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float MeleeAttackDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float MeleeAttackDistance;
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	class UAnimMontage* MeleeAttackMontage;

	void Suicide(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser);
	void KilledByPlayer(struct FDamageEvent const& DamageEvent, class AHorrorCharacter* Character, const AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing = OnRep_Killer)
	class AHorrorCharacter* Killer;

	UFUNCTION()
	void OnRep_Killer();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MulticastDisableCollision();
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency;

	UPROPERTY()
	FInteractionData InteractionData;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckDistance;

	FORCEINLINE class UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	FTimerHandle TimerHandle_Interact;

	UPROPERTY(VisibleAnywhere, Category = "Items")
	TMap<EEquipableSlot, UEquipableItem*> EquippedItems;
public:
	bool IsInteracting() const;

	float GetRemainingInteractTime() const;

	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseItem(class UItem* Item);
	void ServerUseItem_Implementation(class UItem* Item);
	bool ServerUseItem_Validate(class UItem* Item);


	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UItem* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItem(class UItem* Item, const int32 Quantity);
	void ServerDropItem_Implementation(class UItem* Item, const int32 Quantity);
	bool ServerDropItem_Validate(class UItem* Item, const int32 Quantity);

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class APickup> PickupClass;

	UPROPERTY(BlueprintReadOnly,Transient, Replicated)
	bool bIsSprinting;

	
protected:
	
	void PerformInteractionCheck();

	void CouldntFindInteractable();
	void FoundNewInteractable(UInteractionComponent* Interactable);

	void BeginInteract();
	void EndInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();
	void ServerBeginInteract_Implementation();
	bool ServerBeginInteract_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();
	void ServerEndInteract_Implementation();
	bool ServerEndInteract_Validate();


	void Interact();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Restart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditDefaultsOnly, Category=Movement)
	float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category=Movement)
    float CrouchSpeed;

	UPROPERTY(EditDefaultsOnly, Category=Movement)
    float SprintSpeed;

	UPROPERTY(EditDefaultsOnly, Category=Movement)
	float AimSpeed;

	UPROPERTY(EditDefaultsOnly, Category=Movement)
	float AimSpeedCrouched;
	
	void MoveForward(float Val);
	void MoveRight(float Val);

	bool CanSprint();
	
	void StartSprinting();
	void StopSprinting();

	void SetSprinting(const bool bNewSprinting);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(const bool bNewSprinting);
	
	UFUNCTION()
	void BeginLootingPlayer(class AHorrorCharacter* Character);
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetLootSource(class UInventoryComponent* NewLootSource);
	void ServerSetLootSource_Implementation(class UInventoryComponent* NewLootSource);
	bool ServerSetLootSource_Validate(class UInventoryComponent* NewLootSource);

	UPROPERTY(ReplicatedUsing = OnRep_LootSource, BlueprintReadOnly)
	UInventoryComponent* LootSource;

	UFUNCTION()
	void OnLootSourceOwnerDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnRep_LootSource();
	
	bool CanAim() const;

	void StartAiming();
	void StopAiming();
	
	void SetAiming(const bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(const bool bNewAiming);

	UPROPERTY(Transient, Replicated)
	bool bIsAiming;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SetLootSource(class UInventoryComponent* NewLootSource);

	UFUNCTION(BlueprintPure, Category = "Looting")
	bool IsLooting() const;

	UFUNCTION(BlueprintCallable, Category = "Looting")
	void LootItem(class UItem* ItemToGive);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLootItem(class UItem* ItemToGive);
	void ServerLootItem_Implementation(class UItem* ItemToGive);
	bool ServerLootItem_Validate(class UItem* ItemToGive);

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsAlive() const{ return Killer == nullptr;}

	UFUNCTION(BlueprintPure, Category = Weapons)
	FORCEINLINE bool IsAiming() const {return bIsAiming; }
	
};
