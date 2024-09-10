// Fill out your copyright notice in the Description page of Project Settings.


#include "HorrorPlayerController.h"

#include "HorrorCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

AHorrorPlayerController::AHorrorPlayerController() 
{
	MouseSensitivity= 0.25f;
}

void AHorrorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("Turn", this, &AHorrorPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AHorrorPlayerController::LookUp);

	InputComponent->BindAction("Reload", IE_Pressed, this, &AHorrorPlayerController::StartReload);
}

void AHorrorPlayerController::ServerRespawn_Implementation()
{
	Respawn();
}

bool AHorrorPlayerController::ServerRespawn_Validate()
{
	return true;
}

void AHorrorPlayerController::Respawn()
{
	UnPossess();
	ChangeState(NAME_Inactive);
	if(GetLocalRole() < ROLE_Authority)
	{
		ServerRespawn();
	}
	else
	{
		ServerRestartPlayer();
	}
}

void AHorrorPlayerController::ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed,
                                          const float RecoilResetSpeed, TSubclassOf<UCameraShake> Shake)
{
	if (IsLocalPlayerController())
	{
		RecoilBumpAmount += RecoilAmount;
		RecoilResetAmount += -RecoilAmount;

		CurrentRecoilSpeed = RecoilSpeed;
		CurrentRecoilResetSpeed = RecoilResetSpeed;

		LastRecoilTime = GetWorld()->GetTimeSeconds();
	}
}

void AHorrorPlayerController::StartReload()
{
	if(AHorrorCharacter* HorrorCharacter = Cast<AHorrorCharacter>(GetPawn()))
	{
		if(HorrorCharacter->IsAlive())
		{
			HorrorCharacter->StartReload();
		}
		else
		{
			Respawn();
		}
	}
}

void AHorrorPlayerController::LookUp(float Rate)
{
	if(!FMath::IsNearlyZero(RecoilResetAmount.Y, 0.01f))
	{
		if(RecoilResetAmount.Y > 0.f && (Rate * MouseSensitivity) > 0.f)
		{
			RecoilResetAmount.Y = FMath::Max(0.f, RecoilResetAmount.Y- (Rate *MouseSensitivity));
		}
		else if(RecoilResetAmount.Y < 0.f && (Rate *MouseSensitivity) > 0.f)
		{
			RecoilResetAmount.Y = FMath::Min(0.f, RecoilResetAmount.Y - (Rate *MouseSensitivity));
		}
	}
	if(!FMath::IsNearlyZero(RecoilBumpAmount.Y, 0.001f))
	{
		FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.Y = FMath::FInterpTo(RecoilBumpAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

		AddPitchInput(LastCurrentRecoil.Y - RecoilBumpAmount.Y);
	}

	FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.Y = FMath::FInterpTo(RecoilResetAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddPitchInput(LastRecoilResetAmount.Y - RecoilResetAmount.Y);

	AddPitchInput((Rate *MouseSensitivity));
}

void AHorrorPlayerController::Turn(float Rate)
{
	if(FMath::IsNearlyZero(RecoilResetAmount.X, 0.001f))
	{
		if(RecoilResetAmount.X > 0.f && (Rate *MouseSensitivity) > 0.f)
		{
			RecoilResetAmount.X = FMath::Max(0.f, RecoilResetAmount.X - (Rate *MouseSensitivity));
		}
		else if(RecoilResetAmount.X < 0.f && (Rate *MouseSensitivity) < 0.f)
		{
			RecoilResetAmount.X = FMath::Min(0.f, RecoilResetAmount.X- (Rate *MouseSensitivity));
		}
	}
	if(!FMath::IsNearlyZero(RecoilBumpAmount.X, 0.1f))
	{
		FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.X = FMath::FInterpTo(RecoilBumpAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

		AddYawInput(LastCurrentRecoil.X - RecoilBumpAmount.X);
	}
	FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.X = FMath::FInterpTo(RecoilResetAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddYawInput(LastRecoilResetAmount.X - RecoilResetAmount.X);

	AddYawInput((Rate *MouseSensitivity));
}

void AHorrorPlayerController::ClientShowNotification_Implementation(const FText& Message)
{
	ShowNotification(Message);
}
