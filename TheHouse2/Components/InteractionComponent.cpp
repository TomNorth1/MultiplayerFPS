// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "TheHouse2/Player/HorrorCharacter.h"
#include "TheHouse2/Widgets/InteractionWidget.h"

UInteractionComponent::UInteractionComponent() 
{
	SetComponentTickEnabled(false);

	InteractionTime = 0.f;
	InteractionDistance = 200.f;
	InteractableNameText = FText::FromString("Interactable Object");
	InteractableActionText = FText::FromString("Interact");
	bAllowMultipleInteractors = true;

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(400, 100);
	bDrawAtDesiredSize = true;
	bHighlightObject = false;
	
	SetActive(true);
	SetHiddenInGame(true);
}


void UInteractionComponent::BeginFocus(AHorrorCharacter* Character)
{
	if (!IsActive() || !GetOwner() || !Character) 
	{
		return;
	}
	OnBeginFocus.Broadcast(Character);

	SetHiddenInGame(false);
	if (!GetOwner()->HasAuthority()) 
	{
		for (auto& VisualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass())) 
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp)) 
			{
				if(bHighlightObject)
				{
					Prim->SetRenderCustomDepth(true);
				}
			}
		}
	}
	RefreshWidget();
}

void UInteractionComponent::EndFocus(AHorrorCharacter* Character)
{
	OnEndFocus.Broadcast(Character);

	SetHiddenInGame(true);
	if (!GetOwner()->HasAuthority())
	{
		for (auto& VisualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass()))
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(false);
			}
		}
	}
}

void UInteractionComponent::BeginInteract(AHorrorCharacter* Character)
{
	if (CanInteract(Character)) 
	{
		Interactors.AddUnique(Character);
		OnBeginInteract.Broadcast(Character);
	}
}

void UInteractionComponent::EndInteract(AHorrorCharacter* Character)
{
	Interactors.RemoveSingle(Character);
	OnEndInteract.Broadcast(Character);
}

void UInteractionComponent::Interact(AHorrorCharacter* Character)
{
	if (CanInteract(Character)) 
	{
		OnInteract.Broadcast(Character);
	}
}

float UInteractionComponent::GetInteractPercentage()
{
	if (Interactors.IsValidIndex(0)) 
	{
		if (AHorrorCharacter* Interactor = Interactors[0]) 
		{
			if (Interactor && Interactor->IsInteracting()) 
			{
				return 1.f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractionTime);
			}
		}
	}
	return 0.f;
}

void UInteractionComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i) 
	{
		if (AHorrorCharacter* Interactor = Interactors[i]) 
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}

	Interactors.Empty();
}

bool UInteractionComponent::CanInteract(AHorrorCharacter* Character) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UInteractionComponent::SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UInteractionComponent::SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}

void UInteractionComponent::RefreshWidget()
{
	if (!bHiddenInGame && GetOwner()->GetNetMode() != NM_DedicatedServer) 
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject())) 
		{
			InteractionWidget->UpdateInteractionWidget(this);
		}
	}
}
