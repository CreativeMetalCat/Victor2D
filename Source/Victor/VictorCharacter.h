// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "Components/BoxComponent.h"
#include "VictorCharacter.generated.h"

class UTextRenderComponent;

/**
 * This class is the default character for Victor, and it is responsible for all
 * physical interaction between the player and the world.
 *
 * The capsule component (inherited from ACharacter) handles collision with the world
 * The CharacterMovementComponent (inherited from ACharacter) handles movement of the collision capsule
 * The Sprite component (inherited from APaperCharacter) handles the visuals
 */
UCLASS(config=Game)
class AVictorCharacter : public APaperCharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UTextRenderComponent* TextComponent;
	virtual void Tick(float DeltaSeconds) override;
protected:
	// The animation to play while running around
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animations)
	class UPaperFlipbook* RunningAnimation;

	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* IdleAnimation;

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly)
	UBoxComponent* WallGrabBox;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations,SaveGame)
	UPaperFlipbook* StabAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations,SaveGame)
	UPaperFlipbook* DeathAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponAnimations,SaveGame)
	UPaperFlipbook* PistolIdleAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponAnimations,SaveGame)
	UPaperFlipbook* PistolWalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations,SaveGame)
	class UPaperFlipbook* UnPossesAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hold,SaveGame)
	//AHoldableActor* CurrentlyHeldActor = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Death,SaveGame)
	bool bDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Death,SaveGame)
	USoundBase* DeathSound;

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly,Category = Death,SaveGame)
	UAudioComponent* DeathAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=Posses,SaveGame)
	float PossesTime = 1.f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=Weapon,SaveGame)
	//AWeaponBase* Weapon;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=WeaponAnims,SaveGame)
	bool bPlayingMeleeAttackAnim = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=HiddenInShadow,SaveGame)
	bool bHiddenInShadow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=Posses,SaveGame)
	bool bControlledByPlayer = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=Posses,SaveGame)
	AVictorCharacter*OriginalBody = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Climbing,SaveGame)
	bool bIsHoldingWall = false;

	/** Called to choose the correct animation to play based on the character's movement state */
	void UpdateAnimation();

	/** Called for side to side input */
	void MoveRight(float Value);

	void UpdateCharacter();

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	AVictorCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
