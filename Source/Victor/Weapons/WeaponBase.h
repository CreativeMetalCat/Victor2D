// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAnimTypes.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class VICTOR_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
	public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle CooldownTimerHandle;
	public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	float Damage = 10.f;

	//this might be ignored by character. It's more of suggestion rather than demand
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	EWeaponAnimType AnimType = EWeaponAnimType::EWT_MeleeKnife;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	AActor*WeaponOwner;

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category=Cooldown)
	float CooldownTime = 0.f;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category=Cooldown)
	bool bIsCoolingDown = false;

	UFUNCTION(BlueprintPure)
    virtual bool CanShoot();
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category=Sound)
	USoundBase* FireSound;

	UFUNCTION(BlueprintCallable)
    virtual bool Fire(FVector Location,FRotator Rotaion);

	UFUNCTION(BlueprintCallable)
    virtual void OnCooldownEnd();

	UFUNCTION(BlueprintCallable)
    void StartCooldownTimer();

	UFUNCTION(BlueprintCallable)
    virtual void SetHiddenInShadow(bool Hidden){}
};