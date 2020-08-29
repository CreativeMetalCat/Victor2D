// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PossesivePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class VICTOR_API APossesivePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	APawn* OriginalHost;

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly)
	USoundBase* PossesSound;

	virtual void OnChangedBodies();
	
	virtual void BeginPlay() override;
};