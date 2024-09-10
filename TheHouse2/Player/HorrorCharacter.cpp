// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorCharacter.h"

#include "ControlPointMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "TheHouse2/Components/InteractionComponent.h"
#include "TheHouse2/Components/InventoryComponent.h"
#include "TimerManager.h"
#include "Components/SpotLightComponent.h"

#include "TheHouse2/World/Pickup.h"
#include "TheHouse2/Items/EquipableItem.h"
#include "TheHouse2//Items/GearItem.h"
#include "TheHouse2/Player/HorrorPlayerController.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "TheHouse2/Components/InteractionComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnitConversion.h"
#include "TheHouse2/TheHouse2.h"
#include "TheHouse2/Weapons/MeleeDamage.h"
#include "TheHouse2/Items/WeaponItem.h"
#include "TheHouse2/Weapons/Weapon.h"

#define LOCTEXT_NAMESPACE "HorrorCharacter"

static FName NAME_AimDownSightSocket("ADSSocket");

// Sets default values
AHorrorCharacter::AHorrorCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->TargetArmLength = 0.f;
	SpringArmComponent->bUsePawnControlRotation = true;

	FirstPersonSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("FirstPersonSpringArmComponent");
	FirstPersonSpringArmComponent->SetupAttachment(SpringArmComponent);
	FirstPersonSpringArmComponent->TargetArmLength = 0.0f;
	
	
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCastShadow(true);
	
	MeshFirstPerson = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh1PP");
	MeshFirstPerson->SetupAttachment(FirstPersonSpringArmComponent);
	MeshFirstPerson->SetOnlyOwnerSee(true);
	MeshFirstPerson->SetCastShadow(false);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	HelmetMesh = PlayerMeshes.Add(EEquipableSlot::EIS_Helmet, CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HelmetMesh")));

	ArmourMesh = PlayerMeshes.Add(EEquipableSlot::EIS_Armour, CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArmourMesh")));

	BackpackMesh = PlayerMeshes.Add(EEquipableSlot::EIS_Backpack, CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackpackMesh")));

	HelmetMesh->SetupAttachment(GetMesh(), FName("HeadSocket"));
	HelmetMesh->SetOwnerNoSee(true);

	ArmourMesh->SetupAttachment(GetMesh(), FName("ArmourSocket"));
	ArmourMesh->SetOwnerNoSee(true);

	BackpackMesh->SetupAttachment(GetMesh(), FName("BackpackSocket"));
	BackpackMesh->SetOwnerNoSee(true);

	WalkSpeed = 200.f;
	CrouchSpeed = 100.f;
	SprintSpeed = 400.f;
	AimSpeed = 140.f;
	AimSpeedCrouched = 70.f;

	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;

	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetCapacity(8);

	PlayerInteractionComponent = CreateDefaultSubobject<UInteractionComponent>("PlayerInteraction");
	PlayerInteractionComponent->InteractableActionText = LOCTEXT("LootPlayerText", "Loot");
	PlayerInteractionComponent->InteractableNameText = LOCTEXT("LootPlayerName", "Player");
	PlayerInteractionComponent->SetupAttachment(GetRootComponent());
	PlayerInteractionComponent->SetActive(false, true);
	PlayerInteractionComponent->bAutoActivate = false;

	FlashlightPivot= CreateDefaultSubobject<USceneComponent>("FlashlightPivotPoint");
	FlashlightPivot->SetupAttachment(GetMesh());
	
	Flashlight= CreateDefaultSubobject<USpotLightComponent>("Flashlight");
	Flashlight->SetupAttachment(GetMesh(), FName("HeadSocket"));
	Flashlight->SetIsReplicated(true);
	Flashlight->SetVisibility(false);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->JumpZVelocity = 400.f;

	GetCharacterMovement()->AirControl = 0.7f;

	InteractionCheckFrequency = 0.f;
	InteractionCheckDistance = 1000.f;

	MaxHealth = 100.f;
	Health = MaxHealth;
	
	bIsAiming = false;
	bIsSprinting = false;
}



// Called when the game starts or when spawned
void AHorrorCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerInteractionComponent->OnInteract.AddDynamic(this, &AHorrorCharacter::BeginLootingPlayer);

	
	if (APlayerState* PS = GetPlayerState()) 
	{
		PlayerInteractionComponent->SetInteractableNameText(FText::FromString(PS->GetPlayerName()));
	}
	if(IsLocallyControlled())
	{
		Flashlight->AttachToComponent(CameraComponent,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else
	{
		Flashlight->AttachToComponent(GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("FlashlightSocket"));
	}
}

void AHorrorCharacter::Restart()
{
	Super::Restart();
	if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetController()))
	{
		PC->ShowIngameUI();
	}

}

void AHorrorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHorrorCharacter, LootSource);

	DOREPLIFETIME(AHorrorCharacter, Killer);

	DOREPLIFETIME(AHorrorCharacter, Flashlight);
	
	DOREPLIFETIME(AHorrorCharacter, bIsSprinting);
	
	DOREPLIFETIME(AHorrorCharacter, EquippedWeapon);

	DOREPLIFETIME_CONDITION(AHorrorCharacter, bIsAiming, COND_SkipOwner);
	
	DOREPLIFETIME_CONDITION(AHorrorCharacter, Health, COND_OwnerOnly);

}

float AHorrorCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	
	const float DamageDealt = ModifyHealth(-Damage);
	if (Health <= 0.f) 
	{
		if (AHorrorCharacter* KillerCharacter = Cast<AHorrorCharacter>(DamageCauser->GetOwner())) 
		{
			KilledByPlayer(DamageEvent, KillerCharacter, DamageCauser);
		}
		else
		{
			Suicide(DamageEvent, DamageCauser);
		}
	}
	return DamageDealt;
}

UStaticMeshComponent* AHorrorCharacter::GetSlotStaticMeshComponent(const EEquipableSlot Slot)
{
	if (PlayerMeshes.Contains(Slot))
	{
		return *PlayerMeshes.Find(Slot);
	}
	return nullptr;
}

bool AHorrorCharacter::EquipItem(UEquipableItem* Item)
{
	EquippedItems.Add(Item->Slot, Item);
	OnEquipedItemsChanged.Broadcast(Item->Slot, Item);
	return true;
}

bool AHorrorCharacter::UnEquipItem(UEquipableItem* Item)
{
	if (Item) 
	{
		if (EquippedItems.Contains(Item->Slot)) 
		{
			if (Item == *EquippedItems.Find(Item->Slot)) 
			{
				EquippedItems.Remove(Item->Slot);
				OnEquipedItemsChanged.Broadcast(Item->Slot, nullptr);
				return true;
			}
		}
	}
	return false;
}

void AHorrorCharacter::EquipGear(UGearItem* Gear)
{
	if (UStaticMeshComponent* GearMesh = *PlayerMeshes.Find(Gear->Slot)) 
	{
		GearMesh->SetStaticMesh(Gear->Mesh);
		GearMesh->SetOwnerNoSee(true);
	}
}

void AHorrorCharacter::UnEquipGear(UGearItem* Gear)
{
	if (UStaticMeshComponent* GearMesh = *PlayerMeshes.Find(Gear->Slot))
	{
		GearMesh->SetStaticMesh(nullptr);
	}
}

void AHorrorCharacter::EquipWeapon(UWeaponItem* WeaponItem)
{
	if(WeaponItem && WeaponItem->WeaponClass && HasAuthority())
	{
		if(EquippedWeapon)
		{
			UnEquipWeapon();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = this;

		if(AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponItem->WeaponClass, SpawnParams))
		{
			Weapon->Item = WeaponItem;

			EquippedWeapon = Weapon;
			OnRep_EquippedWeapon();

			Weapon->OnEquip();
		}
	}
}

void AHorrorCharacter::UnEquipWeapon()
{
	if(HasAuthority() && EquippedWeapon)
	{
		EquippedWeapon->OnEquip();
		EquippedWeapon->ReturnAmmoToInventory();
		EquippedWeapon->Destroy();
		EquippedWeapon= nullptr;
		OnRep_EquippedWeapon();
	}
}

float AHorrorCharacter::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;

	Health = FMath::Clamp<float>(Health + Delta, 0.f, MaxHealth);

	return Health - OldHealth;
}

void AHorrorCharacter::OnRep_Health(float OldHealth)
{
	OnHealthModified(Health - OldHealth);
}

void AHorrorCharacter::StartReload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartReload();
	}
}


void AHorrorCharacter::ToggleFlashlight()
{
	if (HasAuthority()) 
	{
		Flashlight->ToggleVisibility();
	}
	else 
	{
		Flashlight->ToggleVisibility();
		ServerToggleFlashlight();
	}

}

void AHorrorCharacter::ServerToggleFlashlight_Implementation()
{
	if (HasAuthority()) 
	{
		ToggleFlashlight();
	}
}

void AHorrorCharacter::OnRep_EquippedWeapon()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->OnEquip();
	}
}

void AHorrorCharacter::StartFire()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
	else
	{
		BeginMeleeAttack();
	}
}

void AHorrorCharacter::StopFire()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->StopFire();

	}
}

void AHorrorCharacter::BeginMeleeAttack()
{
	if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength()) 
	{
		FHitResult Hit;
		FCollisionShape Shape = FCollisionShape::MakeSphere(15.f);

		FVector StartTrace = CameraComponent->GetComponentLocation();
		FVector EndTrace = (CameraComponent->GetComponentRotation().Vector() * MeleeAttackDistance) + StartTrace;

		FCollisionQueryParams QueryParams = FCollisionQueryParams("MeleeSwing", false, this);

		PlayAnimMontage(MeleeAttackMontage);

		if (GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat(), COLLISION_WEAPON, Shape, QueryParams)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit"));
			if (AHorrorCharacter* HitPlayer = Cast<AHorrorCharacter>(Hit.GetActor())) 
			{
				if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetController())) 
				{
					PC->OnHitPlayer(Hit);
				}
			}
		}

		ServerProccessMeleeHit(Hit);

		LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
	}
}

void AHorrorCharacter::ServerProccessMeleeHit_Implementation(const FHitResult& MeleeHit)
{
	if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength() && (GetActorLocation() - MeleeHit.ImpactPoint).Size() <= MeleeAttackDistance)
	{
		MulticastPlayMeleeFX();

		UGameplayStatics::ApplyPointDamage(MeleeHit.GetActor(), MeleeAttackDamage, (MeleeHit.TraceStart - MeleeHit.TraceEnd).GetSafeNormal(), MeleeHit, GetController(), this, UMeleeDamage::StaticClass());

	}
	LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
}

void AHorrorCharacter::MulticastPlayMeleeFX_Implementation()
{
	if (!IsLocallyControlled()) 
	{
		PlayAnimMontage(MeleeAttackMontage);
	}
}

void AHorrorCharacter::Suicide(FDamageEvent const& DamageEvent, const AActor* DamageCauser)
{
	Killer = this;
	OnRep_Killer();
}

void AHorrorCharacter::KilledByPlayer(FDamageEvent const& DamageEvent, AHorrorCharacter* Character, const AActor* DamageCauser)
{
	Killer = Character;
	OnRep_Killer();
}

void AHorrorCharacter::OnRep_Killer()
{
	SetLifeSpan(120.f);
	
	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetOwnerNoSee(false);
	MeshFirstPerson->SetHiddenInGame(true);
	MulticastDisableCollision();
	SetReplicateMovement(false);
	
	PlayerInteractionComponent->Activate();

	if (HasAuthority()) 
	{
		TArray<UEquipableItem*> Equippables;
		EquippedItems.GenerateValueArray(Equippables);

		for (auto& EquippedItem : Equippables) 
		{
			EquippedItem->SetEquipped(false);
		}
	}

	if (IsLocallyControlled()) 
	{
		SpringArmComponent->TargetArmLength = 500.f;
		bUseControllerRotationPitch = true;
		if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetController())) 
		{
			PC->ShowDeathScreen(Killer);
		}
	}
}

void AHorrorCharacter::MulticastDisableCollision_Implementation()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetSimulatePhysics(true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

bool AHorrorCharacter::MulticastDisableCollision_Validate()
{
	return  true;
}

void AHorrorCharacter::MoveForward(float Val)
{
	AddMovementInput(GetActorForwardVector(), Val);
}

void AHorrorCharacter::MoveRight(float Val)
{
	AddMovementInput(GetActorRightVector(), Val);
}

bool AHorrorCharacter::CanSprint()
{
	return bIsCrouched == false && GetCharacterMovement()->IsFalling() == false && IsAiming() == false;
}

void AHorrorCharacter::StartSprinting()
{
	if(CanSprint())
	{
		SetSprinting(true);
	}
}

void AHorrorCharacter::StopSprinting()
{
	SetSprinting(false);	
}

void AHorrorCharacter::SetSprinting(const bool bNewSprinting)
{
	if ((bNewSprinting && !CanSprint()) || bNewSprinting == bIsSprinting)
	{
		return;
	}

	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetSprinting(bNewSprinting);
	}

	bIsSprinting = bNewSprinting;

	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	
}

bool AHorrorCharacter::ServerSetSprinting_Validate(const bool bNewSprinting)
{
	return true;
}

void AHorrorCharacter::ServerSetSprinting_Implementation(const bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}


// Called every frame
void AHorrorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());


	if ((!HasAuthority() || bIsInteractingOnServer) && GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency) 
	{
		PerformInteractionCheck();
	}

	
	if(IsLocallyControlled())
	{
		const float DesiredFOV = IsAiming() ? 70.f : 100.f;
		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, DesiredFOV, DeltaTime, 10.f));

		if(EquippedWeapon)
		{
			const FVector ADSLocation = MeshFirstPerson->GetSocketLocation("CameraSocket");
			const FVector DefaultCameraLocation = SpringArmComponent->GetComponentLocation();
			FVector CameraLoc = bIsAiming ? ADSLocation : DefaultCameraLocation;

			
			
			const float InterpSpeed = FVector::Dist(ADSLocation, DefaultCameraLocation) / EquippedWeapon->ADSTime;
			//CameraComponent->SetWorldLocation(FMath::VInterpTo(CameraComponent->GetComponentLocation(), CameraLoc, DeltaTime, InterpSpeed));
			if(Flashlight->IsVisible())
			{
				Flashlight->SetWorldLocation(EquippedWeapon->WeaponMesh->GetSocketLocation("Flashlight"));
				Flashlight->SetWorldRotation(EquippedWeapon->WeaponMesh->GetSocketRotation("Flashlight"));
			}
		}
		if(Flashlight->IsVisible() && !EquippedWeapon)
		{
			Flashlight->SetWorldLocation(CameraComponent->GetComponentLocation());
			Flashlight->SetWorldRotation(CameraComponent->GetComponentRotation());
		}
	}
}




void AHorrorCharacter::PerformInteractionCheck()
{
	if (GetController() == nullptr) 
	{
		return;
	}

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();
	FVector EyesLoc;
	FRotator EyesRot;

	GetController()->GetPlayerViewPoint(EyesLoc, EyesRot);

	FVector TraceStart = EyesLoc;
	FVector TraceEnd = (EyesRot.Vector() * InteractionCheckDistance) + TraceStart;
	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)) 
	{
		if (TraceHit.GetActor()) 
		{
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size();
				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance) 
				{
					FoundNewInteractable(InteractionComponent);
				}
				else if (Distance > InteractionComponent->InteractionDistance && GetInteractable()) 
				{
					CouldntFindInteractable();
				}
				return;
			}
		}
	}
	CouldntFindInteractable();
}

void AHorrorCharacter::CouldntFindInteractable()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact)) 
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}
	if (UInteractionComponent* Interactable = GetInteractable()) 
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld) 
		{
			EndInteract();
		}
	}
	InteractionData.ViewedInteractionComponent = nullptr;
	
}

void AHorrorCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	EndInteract();

	if (UInteractionComponent* OldInteractable = GetInteractable()) 
	{
		OldInteractable->EndFocus(this);
	}


	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);
}

void AHorrorCharacter::BeginInteract()
{
	if (!HasAuthority()) 
	{
		ServerBeginInteract();
	}

	if (HasAuthority()) 
	{
		PerformInteractionCheck();
	}

	InteractionData.bInteractHeld = true;

	if (UInteractionComponent* Interactable = GetInteractable()) 
	{
		Interactable->BeginInteract(this);
		if (FMath::IsNearlyZero(Interactable->InteractionTime)) 
		{
			Interact();
		}
		else 
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &AHorrorCharacter::Interact, Interactable->InteractionTime, false);
		}
	}
}

void AHorrorCharacter::EndInteract()
{
	if (!HasAuthority()) 
	{
		ServerEndInteract();
	}
	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	if (UInteractionComponent* Interactable = GetInteractable()) 
	{
		Interactable->EndInteract(this);
	}
}

void AHorrorCharacter::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool AHorrorCharacter::ServerBeginInteract_Validate() 
{
	return true;
}

void AHorrorCharacter::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool AHorrorCharacter::ServerEndInteract_Validate()
{
	return true;
}

void AHorrorCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	if (UInteractionComponent* Interactable = GetInteractable()) 
	{
		Interactable->Interact(this);
	}
}

bool AHorrorCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}
float AHorrorCharacter::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void AHorrorCharacter::UseItem(UItem* Item)
{
	if (GetLocalRole() < ROLE_Authority && Item) 
	{
		ServerUseItem(Item);
	}

	if (HasAuthority()) 
	{
		if (PlayerInventory && !PlayerInventory->FindItem(Item)) 
		{
			return;
		}
	}

	if (Item) 
	{
		Item->Use(this);
	}
}

void AHorrorCharacter::DropItem(UItem* Item, const int32 Quantity)
{
	if (PlayerInventory && Item && PlayerInventory->FindItem(Item))
	{
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerDropItem(Item, Quantity);
			return;
		}

		if (HasAuthority())
		{
			const int32 ItemQuantity = Item->GetQuantity();
			const int32 DroppedQuantity = PlayerInventory->ConsumeItem(Item, Quantity);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector SpawnLocation = GetActorLocation();
			SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

			ensure(PickupClass);

			if (APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams))
			{
				Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
			}
		}
	}
}

void AHorrorCharacter::ServerUseItem_Implementation(UItem* Item)
{
	UseItem(Item);
}

bool AHorrorCharacter::ServerUseItem_Validate(UItem* Item)
{
	return true;
}

void AHorrorCharacter::ServerDropItem_Implementation(UItem* Item, const int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool AHorrorCharacter::ServerDropItem_Validate(UItem* Item, const int32 Quantity)
{
	return true;
}



/*
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Looting

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void AHorrorCharacter::BeginLootingPlayer(AHorrorCharacter* Character)
{
	if (Character) 
	{
		Character->SetLootSource(PlayerInventory);
	}
}


void AHorrorCharacter::ServerSetLootSource_Implementation(UInventoryComponent* NewLootSource)
{
	SetLootSource(NewLootSource);
}

bool AHorrorCharacter::ServerSetLootSource_Validate(UInventoryComponent* NewLootSource)
{
	return true;
}

void AHorrorCharacter::OnLootSourceOwnerDestroyed(AActor* DestroyedActor)
{
	if (HasAuthority() && LootSource && DestroyedActor == LootSource->GetOwner()) 
	{
		ServerSetLootSource(nullptr);
	}
}

void AHorrorCharacter::OnRep_LootSource()
{
	if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetController())) 
	{
		if (PC->IsLocalController()) 
		{
			if (LootSource) 
			{
				PC->ShowLootMenu(LootSource);
			}
			else 
			{
				PC->HideLootMenu();
			}
		}
	}
}

bool AHorrorCharacter::CanAim() const
{
	return EquippedWeapon != nullptr && GetCharacterMovement()->IsFalling() == false && !bIsSprinting;
}

void AHorrorCharacter::StartAiming()
{
	if(CanAim())
	{
		SetAiming(true);
	}
}

void AHorrorCharacter::StopAiming()
{
	SetAiming(false);
}

void AHorrorCharacter::SetAiming(const bool bNewAiming)
{
	if((bNewAiming && !CanAim()) || bNewAiming == bIsAiming)
	{
		return;
	}
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerSetAiming(bNewAiming);
	}
	bIsAiming = bNewAiming;
				
	GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = bIsAiming && bIsCrouched ? AimSpeedCrouched : CrouchSpeed;
		
}

void AHorrorCharacter::ServerSetAiming_Implementation(const bool bNewAiming)
{
	SetAiming(bNewAiming);
}

void AHorrorCharacter::SetLootSource(UInventoryComponent* NewLootSource)
{
	if (NewLootSource && NewLootSource->GetOwner())
	{
		NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &AHorrorCharacter::OnLootSourceOwnerDestroyed);
	}

	if (HasAuthority())
	{
		if (NewLootSource) 
		{
			if (AHorrorCharacter* Character = Cast<AHorrorCharacter>(NewLootSource->GetOwner())) 
			{
				Character->SetLifeSpan(120.f);
			}
		}

		LootSource = NewLootSource;
	}
	else 
	{
		ServerSetLootSource(NewLootSource);
	}
}

bool AHorrorCharacter::IsLooting() const
{
	return LootSource != nullptr;
}

void AHorrorCharacter::LootItem(UItem* ItemToGive)
{
	if (HasAuthority()) 
	{
		if (PlayerInventory && LootSource && ItemToGive && LootSource->HasItem(ItemToGive->GetClass(), ItemToGive->GetQuantity())) 
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToGive);

			if (AddResult.ActualAmountGiven > 0) 
			{
				LootSource->ConsumeItem(ItemToGive, AddResult.ActualAmountGiven);
			}
			else 
			{
				if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetController())) 
				{
					PC->ClientShowNotification(AddResult.ErrorText);
				}
			}
		}
	}
	else 
	{
		ServerLootItem(ItemToGive);
	}
}

void AHorrorCharacter::ServerLootItem_Implementation(UItem* ItemToGive)
{
	LootItem(ItemToGive);
}

bool AHorrorCharacter::ServerLootItem_Validate(UItem* ItemToGive)
{
	return true;
}

// Called to bind functionality to input
void AHorrorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHorrorCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHorrorCharacter::MoveRight);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHorrorCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHorrorCharacter::StopSprinting);
	//PlayerInputComponent->BindAxis("LookUp", this, &AHorrorCharacter::FlashlightPitch);
	
	
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AHorrorCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AHorrorCharacter::EndInteract);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHorrorCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AHorrorCharacter::StopFire);
	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &AHorrorCharacter::StartAiming);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &AHorrorCharacter::StopAiming);
	PlayerInputComponent->BindAction("Flashlight", IE_Pressed, this, &AHorrorCharacter::ToggleFlashlight);
}


#undef LOCTEXT_NAMESPACE 