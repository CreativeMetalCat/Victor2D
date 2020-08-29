// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "Components/BoxComponent.h"
#include "Weapons/WeaponBase.h"
#include "Components/AudioComponent.h"
#include "VictorCharacter.generated.h"

UENUM(BlueprintType)
enum class ETeam:uint8
{
	ET_Player UMETA(DisplayName = "Player"),
    ET_Guards UMETA(DisplayName = "Guards")
};

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

	FTimerHandle StartPossesingTimerHandle;

	FTimerHandle FinishAttackAnimTimerHandle;

	FTimerHandle MeleeDealDamageTimerHandle;

	FTimerHandle EndMeleeAttackAnimTimerHandle;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category=Weapon,SaveGame)
	AWeaponBase* Weapon;	

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

	UFUNCTION(BlueprintCallable)
	virtual bool SetWeapon(TSubclassOf<AWeaponBase>WeaponClass);

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


	UFUNCTION(BlueprintPure)
	virtual bool CanBePossesed(){return true;}

	UFUNCTION(BLueprintPure)
	virtual bool CanBeSeen();
	
	void Interact();

	UFUNCTION(BlueprintCallable)
	virtual void Die();

	UFUNCTION(BlueprintCallable)
	virtual void SetHiddenInTheShadow(bool Hidden);
	
	UFUNCTION(BlueprintPure)
	virtual UPaperFlipbook* GetDesiredAnimation();

	UFUNCTION(BlueprintPure)
	FVector GetWeaponSocketLocation()const;

	UFUNCTION(BlueprintPure)
	FName GetWeaponAttachmentSocketName(EWeaponAnimType animType)const;

	UFUNCTION(BlueprintPure)
    FRotator GetWeaponSocketRotation()const;

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category= SaveSystem)
	void LoadLastSave();
	
	void LoadLastSave_Implementation(){}
	
	
	virtual void Attack();

	virtual void EndMeleeAttackAnim();
	
	virtual void FinishMeleeAttack();

	virtual void BeginDestroy() override;
	
	
	void Possess();

	void StartPossess();

	void StopPossess();
	

	virtual void OnUnPosses();

	virtual void OnPosses(AVictorCharacter *originalBody);
	

	virtual void BeginPlay() override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual bool CanJumpInternal_Implementation() const override;

	UFUNCTION()
    void OnWallGrabBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
    void OnWallGrabBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	AVictorCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
