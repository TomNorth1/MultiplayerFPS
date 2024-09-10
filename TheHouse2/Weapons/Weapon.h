// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectMacros.h"
#include "Weapon.generated.h"

class UAnimMontage;
class UAnimationAsset;
class AHorrorCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;

UENUM(BlueprintType)
enum class EWeaponState: uint8
{
	Idle,
	Firing,
	Reloading,
	Equipping
};

UENUM(BlueprintType, Category = Weapon)
enum class EWeaponType : uint8
{
	Unarmed,
	M4,
	AK,
	Pistol,
	Deagle,
	Shotgun,
	Sniper,
	SMG,
	M249
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ammo)
	int32 AmmoPerClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= Ammo)
	TSubclassOf<class UAmmoItem> AmmoClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= WeaponSlot)
	float TimeBetweenShots;
	FWeaponData()
	{
		AmmoPerClip = 30;
		TimeBetweenShots = 0.13f;
	}
};

USTRUCT()
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditDefaultsOnly,Category = Animation)
	UAnimMontage* Pawn1P;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* Pawn3P;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* Weapon1PP;
};

USTRUCT(BlueprintType)
struct FHitScanConfiguration
{
	GENERATED_BODY()
	FHitScanConfiguration()
	{
		Distance = 10000.f;
		Damage = 25.f;
		Radius = 0.f;
		DamageType = UDamageType::StaticClass();
	}
	UPROPERTY(EditDefaultsOnly,Category="Trace Info")
	TMap<FName, float> BoneDamageModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Trace Info")
	float Distance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Trace Info")
	float Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Trace Info")
	float Radius;

	UPROPERTY(EditDefaultsOnly, Category= "Weapon Stat")
	TSubclassOf<UDamageType> DamageType;
};

UCLASS()
class THEHOUSE2_API AWeapon : public AActor
{
	GENERATED_BODY()
	friend class AHorrorCharacter;
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

protected:
	void UseClipAmmo();

	void ConsumeAmmo(const int32 Amount);

	void ReturnAmmoToInventory();

	virtual void OnEquip();

	virtual void OnEquipFinished();

	virtual  void OnUnEquip();

	bool IsEquipped() const;

	bool IsAttachedToPawn() const;

	virtual void StartFire();

	virtual void StopFire();

	virtual void StartReload(bool bFromReplication = false);

	virtual void StopReload();

	virtual void ReloadWeapon();

	UFUNCTION(Reliable, Client)
	void ClientStartReload();

	bool CanFire() const;
	bool CanReload() const;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	int32 GetCurrentAmmoInClip() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	EWeaponState GetCurrentState() const;

	int32 GetAmmoPerClip() const;

	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category="Weapon")
	class AHorrorCharacter* GetPawnOwner() const;

	void SetPawnOwner(AHorrorCharacter* HorrorCharacter);

	float GetEquipStartedTime() const;

	float GetEquipDuration() const;

	UPROPERTY(Replicated, BlueprintReadOnly, Transient)
	class UWeaponItem* Item;

	UPROPERTY(Transient, ReplicatedUsing= OnRep_PawnOwner)
	class AHorrorCharacter* PawnOwner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Config)
	FWeaponData WeaponConfig;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Config)
	FHitScanConfiguration HitScanConfig;
	
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Config)
	TSubclassOf<class AHorrorProjectile> ProjectileClass;*/

public:
	
	UPROPERTY(EditAnywhere, Category = Components)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Weapon)
	EWeaponType WeaponType;
	
protected:
	
	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	UPROPERTY(Config)
	bool bAllowAutomaticWeaponCatchup = true;

	UPROPERTY(Transient)
	UAudioComponent* FireAC;
	
	UPROPERTY(EditDefaultsOnly, Category= Effects)
	FName MuzzleAttachPoint;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	FName AttachSocket;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
    FName AttachSocket1PP;
	
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* MuzzleFX;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSCSecondary;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<UCameraShake> FireCameraShake;
	
	UPROPERTY(EditDefaultsOnly, Category=Weapon)
	float ADSTime;

	UPROPERTY(EditDefaultsOnly, Category=Recoil)
	class UCurveVector* RecoilCurve;

	UPROPERTY(EditDefaultsOnly, Category = Recoil)
	float RecoilSpeed;

	UPROPERTY(EditDefaultsOnly, Category=Recoil)
	float RecoilResetSpeed;

	UPROPERTY(EditDefaultsOnly, Category= Effects)
	UForceFeedbackEffect *FireForceFeedback;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundAttenuation* FireAttenuation;
	
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireLoopSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireFinishSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* OutOfAmmoSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim ReloadAnim;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* EquipSound;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim FireAnim;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim FireAimingAnim;

	UPROPERTY(EditDefaultsOnly, Category = Effects)
    uint32 bLoopedMuzzleFX : 1;

	/** is fire sound looped? */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
    uint32 bLoopedFireSound : 1;

	/** is fire animation looped? */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
    uint32 bLoopedFireAnim : 1;

	/** is fire animation playing? */
	uint32 bPlayingFireAnim : 1;

	/** is weapon currently equipped? */
	uint32 bIsEquipped : 1;

	/** is weapon fire active? */
	uint32 bWantsToFire : 1;

	/** is reload animation playing? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
    uint32 bPendingReload : 1;

	/** is equip animation playing? */
	uint32 bPendingEquip : 1;

	/** weapon is refiring */
	uint32 bRefiring;

	/** current weapon state */
	EWeaponState CurrentState;

	/** time of last successful weapon fire */
	float LastFireTime;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	/** current ammo - inside clip */
	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	/** burst counter, used for replicating fire events to remote clients */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	/** Handle for efficient management of OnEquipFinished timer */
	FTimerHandle TimerHandle_OnEquipFinished;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_StopReload;

	/** Handle for efficient management of ReloadWeapon timer */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle for efficient management of HandleFiring timer */
	FTimerHandle TimerHandle_HandleFiring;

	//////////////////////////////////////////////////////////////////////////
// Input - server side

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartReload();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopReload();

	//////////////////////////////////////////////////////////////////////////
// Replication & effects
	UFUNCTION()
	void OnRep_PawnOwner();

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Reload();

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();

	//////////////////////////////////////////////////////////////////////////
// Weapon usage


	/**Handle hit locally before asking server to process hit*/
	void HandleHit(const FHitResult& Hit, class AHorrorCharacter* HitPlayer = nullptr);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandleHit(const FHitResult& Hit, class AHorrorCharacter* HitPlayer = nullptr);

	/** [local] weapon specific fire implementation */
	virtual void FireShot();

	/** [server] fire & update ammo */
	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	/** [local + server] handle weapon refire, compensating for slack time if the timer can't sample fast enough */
	void HandleReFiring();

	/** [local + server] handle weapon fire */
	void HandleFiring();

	/** [local + server] firing started */
	virtual void OnBurstStarted();

	/** [local + server] firing finished */
	virtual void OnBurstFinished();

	/** update weapon state */
	void SetWeaponState(EWeaponState NewState);

	/** determine current weapon state */
	void DetermineWeaponState();

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage helpers

	/** play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	/** play weapon animations */
	float PlayWeaponAnimation(const FWeaponAnim& Animation);

	/** stop playing weapon animations */
	void StopWeaponAnimation(const FWeaponAnim& Animation);

	/** Get the aim of the camera */
	FVector GetCameraAim() const;

	/** find hit */
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const;
};
