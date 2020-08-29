// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AWeaponBase::CanShoot()
{
	return  !bIsCoolingDown;
}

bool AWeaponBase::Fire(FVector Location,FRotator Rotaion)
{
	if(CanShoot())
	{
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, Location, Rotaion);
		}
		StartCooldownTimer();
	}
	return false;
}

void AWeaponBase::OnCooldownEnd()
{
	bIsCoolingDown = false;
}

void AWeaponBase::StartCooldownTimer()
{
	if(CooldownTime>0.f)
	{
		bIsCoolingDown = true;
		GetWorldTimerManager().SetTimer(CooldownTimerHandle,this,&AWeaponBase::OnCooldownEnd,CooldownTime);
	}
}

