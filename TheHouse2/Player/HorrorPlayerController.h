// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HorrorCharacter.h"
#include "GameFramework/PlayerController.h"
#include "HorrorPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THEHOUSE2_API AHorrorPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AHorrorPlayerController();
	
	UPROPERTY(BlueprintReadWrite, Category=Controls)
	float MouseSensitivity;
	
	virtual void SetupInputComponent() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowIngameUI();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientShowNotification(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowNotification(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDeathScreen(class AHorrorCharacter* Killer);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowLootMenu(const class UInventoryComponent* LootSource);
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideLootMenu();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHitPlayer(const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category="Horror Controller")
	void Respawn();
	
	UFUNCTION(Server, Reliable,WithValidation)
	void ServerRespawn();
	
	void ApplyRecoil(const FVector2D& RecoilAmmount, const float RecoilSpeed, const float RecoilResetSpeed, TSubclassOf<class UCameraShake> Shake = nullptr);

	UPROPERTY(VisibleAnywhere,Category=Recoil)
	FVector2D RecoilBumpAmount;

	UPROPERTY(VisibleAnywhere, Category=Recoil)
	FVector2D RecoilResetAmount;

	UPROPERTY(EditDefaultsOnly, Category=Recoil)
	float CurrentRecoilSpeed;

	UPROPERTY(EditDefaultsOnly, Category=Recoil)
	float CurrentRecoilResetSpeed;

	UPROPERTY(VisibleAnywhere, Category=Recoil)
	float LastRecoilTime;

	void Turn(float Rate);
	void LookUp(float Rate);

	void StartReload();
	
};
