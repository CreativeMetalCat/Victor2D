// Fill out your copyright notice in the Description page of Project Settings.


#include "PossesivePlayerController.h"

#include "Kismet/GameplayStatics.h"

void APossesivePlayerController::OnChangedBodies()
{
    if (PossesSound != nullptr)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), PossesSound);
    }
}

void APossesivePlayerController::BeginPlay()
{
    OriginalHost = GetPawn();
    
    FInputModeGameAndUI inputMode = FInputModeGameAndUI();
    inputMode.SetHideCursorDuringCapture(false);
    SetInputMode(FInputModeGameAndUI());

    bShowMouseCursor = true;
}