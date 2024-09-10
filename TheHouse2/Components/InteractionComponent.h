// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class AHorrorCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class AHorrorCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class AHorrorCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class AHorrorCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class AHorrorCharacter*, Character);
/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THEHOUSE2_API UInteractionComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
    UInteractionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
    float InteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
    float InteractionDistance;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
    FText InteractableNameText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
    FText InteractableActionText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
    bool bAllowMultipleInteractors;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction)
    bool bHighlightObject;
    
    UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
    FOnBeginInteract OnBeginInteract;

    UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
    FOnEndInteract OnEndInteract;

    UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
    FOnBeginFocus OnBeginFocus;

    UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
    FOnEndFocus OnEndFocus;

    UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
    FOnInteract OnInteract;

    //FUNCTIONS

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractableNameText(const FText& NewNameText);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractableActionText(const FText& NewActionText);

    void RefreshWidget();

    void BeginFocus(class AHorrorCharacter* Character);
    void EndFocus(class AHorrorCharacter* Character);

    void BeginInteract(class AHorrorCharacter* Character);
    void EndInteract(class AHorrorCharacter* Character);
     
    void Interact(class AHorrorCharacter* Character);

    UFUNCTION(BlueprintPure, Category = "Interaction")
    float GetInteractPercentage();

protected:
    virtual void Deactivate() override;

    bool CanInteract(class AHorrorCharacter* Character) const;

    UPROPERTY()
    TArray<class AHorrorCharacter*> Interactors;
};


