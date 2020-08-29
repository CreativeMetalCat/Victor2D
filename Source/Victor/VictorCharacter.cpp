// Copyright Epic Games, Inc. All Rights Reserved.

#include "VictorCharacter.h"

#include "Interactions.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "Player/PossesivePlayerController.h"


DEFINE_LOG_CATEGORY_STATIC(SideScrollerCharacter, Log, All);

//////////////////////////////////////////////////////////////////////////
// AVictorCharacter

AVictorCharacter::AVictorCharacter()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 75.0f);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 2048.0f;
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->SetUsingAbsoluteRotation(true);
	SideViewCameraComponent->bUsePawnControlRotation = false;
	SideViewCameraComponent->bAutoActivate = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 800.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	DeathAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("DeathAudio"));
	DeathAudio->SetupAttachment(RootComponent);
	DeathAudio->bAutoActivate = false;

	WallGrabBox=CreateDefaultSubobject<UBoxComponent>(TEXT("WallGrabBox"));
	WallGrabBox->SetupAttachment(RootComponent);
	WallGrabBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WallGrabBox->SetBoxExtent(FVector(24,32,8));
	WallGrabBox->SetRelativeLocation(FVector(50,0,20));

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

    // 	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
    // 	TextComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
    // 	TextComponent->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
    // 	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    // 	TextComponent->SetupAttachment(RootComponent);

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;
}

//////////////////////////////////////////////////////////////////////////
// Animation

bool AVictorCharacter::SetWeapon(TSubclassOf<AWeaponBase> WeaponClass)
{
	Weapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
	if(Weapon != nullptr)
	{
		Weapon->AttachToComponent(GetSprite(),FAttachmentTransformRules::SnapToTargetNotIncludingScale, GetWeaponAttachmentSocketName(Weapon->AnimType));
		Weapon->WeaponOwner = this;
		return true;
	}
	return false;
}

void AVictorCharacter::UpdateAnimation()
{
	if(!bDead && !bPlayingMeleeAttackAnim)
	{
		// Are we moving or standing still?
		UPaperFlipbook* DesiredAnimation = GetDesiredAnimation(); 
		if( GetSprite()->GetFlipbook() != DesiredAnimation 	)
		{
			GetSprite()->SetFlipbook(DesiredAnimation);
		}
		if (!GetSprite()->IsLooping()) { GetSprite()->SetLooping(true); GetSprite()->PlayFromStart(); }
	}
}

void AVictorCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateCharacter();	
}


//////////////////////////////////////////////////////////////////////////
// Input

void AVictorCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVictorCharacter::MoveRight);

	PlayerInputComponent->BindAction("Interact",IE_Pressed,this,&AVictorCharacter::Interact);

	PlayerInputComponent->BindAction("Possess",IE_Pressed,this,&AVictorCharacter::StartPossess);
	PlayerInputComponent->BindAction("Possess",IE_Released,this,&AVictorCharacter::StopPossess);
	
	PlayerInputComponent->BindAction("Attack",IE_Pressed,this,&AVictorCharacter::Attack);
}

bool AVictorCharacter::CanBeSeen()
{
	return !(bDead || bHiddenInShadow);
}

void AVictorCharacter::Interact()
{
	TArray<AActor*> actors;
	GetCapsuleComponent()->GetOverlappingActors(actors);
	if (actors.Num() > 0)
	{
		for (int i = 0; i < actors.Num(); i++)
		{
			if (actors[i]->Implements<UInteractions>() || (Cast<IInteractions>(actors[i]) != nullptr))
			{
				IInteractions::Execute_Interact(actors[i], this);
			}
		}
	}
}

void AVictorCharacter::Die()
{
	if (!bDead)
	{
		APossesivePlayerController*PC = Cast<APossesivePlayerController>(GetController());
		if ( PC != nullptr)
		{
			DisableInput(PC);
			LoadLastSave();//for testing only.
			//TODO: Remove this after testing
		}
		bDead = true;
		if (DeathAnimation != nullptr)
		{
			GetSprite()->SetFlipbook(DeathAnimation);
			GetSprite()->SetLooping(false);
		}
		if(!DeathAudio->IsPlaying())
		{
			DeathAudio->Play();
		}
		if (GetController() != nullptr)
		{
			if(Cast<APlayerController>(GetController()) == nullptr)
			{
				GetController()->UnPossess();
			}
		}
		if(Weapon != nullptr)
		{
			Weapon->Destroy();
			Weapon = nullptr;
		}
	}
}

void AVictorCharacter::SetHiddenInTheShadow(bool Hidden)
{
	bHiddenInShadow = Hidden;
	if (Weapon != nullptr)
	{
		Weapon->SetHiddenInShadow(Hidden);
	}
	if (Hidden)
	{
		GetSprite()->SetSpriteColor(FColor::Black);
	}
	else
	{
		GetSprite()->SetSpriteColor(FColor::White);
	}
}

UPaperFlipbook* AVictorCharacter::GetDesiredAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();
	
	if(Weapon != nullptr)
	{
		switch (Weapon->AnimType)
		{
		case EWeaponAnimType::EWT_Pistol:
			return (PlayerSpeedSqr > 0.0f) ? PistolWalkAnimation : PistolIdleAnimation;
			break;
			
		case EWeaponAnimType::EWT_MeleeKnife:
			return (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
			break;
			
		default:
			return (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
			break;
		}
	}
	else
	{
		
		return (PlayerSpeedSqr > 0.0f) ? RunningAnimation : IdleAnimation;
	}
}

FVector AVictorCharacter::GetWeaponSocketLocation() const
{
	if(Weapon != nullptr)
	{
		return GetSprite()->GetSocketLocation(GetWeaponAttachmentSocketName(Weapon->AnimType));
	}
	else
	{
		return GetSprite()->GetSocketLocation(TEXT("WeaponHolding"));
	}
}

FRotator AVictorCharacter::GetWeaponSocketRotation() const
{
	if(Weapon != nullptr)
	{
		return GetSprite()->GetSocketRotation(GetWeaponAttachmentSocketName(Weapon->AnimType));
	}
	else
	{
		return GetSprite()->GetSocketRotation(TEXT("WeaponHolding"));
	}
}

FName AVictorCharacter::GetWeaponAttachmentSocketName(EWeaponAnimType animType)const
{
	switch (animType)
	{
	case EWeaponAnimType::EWT_Pistol:
		return TEXT("PistolHolding");
		break;
	case EWeaponAnimType::EWT_MeleeKnife:
		return TEXT("WeaponHolding");
		break;
	default:
		return TEXT("WeaponHolding");
		break;
	}
}

void AVictorCharacter::Attack()
{
	/*if(!bHiddenInShadow)
	{
		if(Weapon != nullptr)
		{
			if(Cast<AKnifeBase>(Weapon) != nullptr)
			{
				if(!EndMeleeAttackAnimTimerHandle.IsValid())
				{
					if(StabAnimation != nullptr)
					{
						GetSprite()->SetLooping(false);
						bPlayingMeleeAttackAnim = true;
						GetSprite()->SetFlipbook(StabAnimation);
						GetSprite()->PlayFromStart();
						GetWorldTimerManager().SetTimer(EndMeleeAttackAnimTimerHandle,this,&AVictorCharacter::EndMeleeAttackAnim,GetSprite()->GetFlipbookLength());
					}
					else
					{
						Cast<AKnifeBase>(Weapon)->DealDamage();
					}
				}
			}
			else
			{
				Weapon->Fire(GetWeaponSocketLocation(),GetWeaponSocketRotation());
			}
		}
	}*/
}

void AVictorCharacter::EndMeleeAttackAnim()
{
	GetWorldTimerManager().ClearTimer(EndMeleeAttackAnimTimerHandle);
	if(StabAnimation != nullptr)
	{
		//Cast<AKnifeBase>(Weapon)->DealDamage();
		GetSprite()->ReverseFromEnd();
		GetWorldTimerManager().SetTimer(FinishAttackAnimTimerHandle,this,&AVictorCharacter::FinishMeleeAttack,GetSprite()->GetFlipbookLength());
	}
}

void AVictorCharacter::FinishMeleeAttack()
{
	bPlayingMeleeAttackAnim = false;
}

void AVictorCharacter::BeginDestroy()
{
	Super::BeginDestroy();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if(Weapon != nullptr)
	{
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AVictorCharacter::Possess()
{
	GetWorldTimerManager().ClearTimer(StartPossesingTimerHandle);
	if (GetController() != nullptr)
	{
		APossesivePlayerController*PC = Cast<APossesivePlayerController>(GetController());
		if ( PC != nullptr)
		{
			if(OriginalBody != nullptr && OriginalBody->Tags.Find("Player") != -1)
			{
				OnUnPosses();
				OriginalBody->OnPosses(this);
				PC->OnChangedBodies();
				PC->Possess(OriginalBody);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Emerald,"Shooting line");
				FHitResult hit;
				PC->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery2,false,hit);
				if(hit.bBlockingHit)
				{
					if(hit.Actor != nullptr)
					{
						AVictorCharacter* Other = Cast<AVictorCharacter>(hit.GetActor());
						if(Other!=nullptr)
						{
							if(Other->CanBePossesed())
							{
								OnUnPosses();
								Other->OnPosses(this);
								PC->OnChangedBodies();
								PC->Possess(Other);
							}
						}
						else
						{
							GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Yellow,"Actor is not possesable. Actor name: " + hit.GetActor()->GetName());
						
						}
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Emerald,"Hit but found no actor");
					}
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,"Didn't hit");
					GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,hit.TraceStart.ToString());
				}
			}
		}
	}
}

void AVictorCharacter::StartPossess()
{
	GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Emerald,"Starting...");
	if(!StartPossesingTimerHandle.IsValid())
	{
		GetWorldTimerManager().SetTimer(StartPossesingTimerHandle,this,&AVictorCharacter::Possess,PossesTime);
	}
}

void AVictorCharacter::StopPossess()
{
	GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Emerald,"Aborting...");
	GetWorldTimerManager().ClearTimer(StartPossesingTimerHandle);
}

void AVictorCharacter::OnUnPosses()
{
	bControlledByPlayer = false;
}

void AVictorCharacter::OnPosses(AVictorCharacter*originalBody)
{
	OriginalBody = originalBody;
	bControlledByPlayer = true;
}

void AVictorCharacter::BeginPlay()
{
	Super::BeginPlay();

	WallGrabBox->OnComponentBeginOverlap.AddDynamic(this, &AVictorCharacter::OnWallGrabBoxBeginOverlap);

	WallGrabBox->OnComponentEndOverlap.AddDynamic(this, &AVictorCharacter::OnWallGrabBoxEndOverlap);
}

float AVictorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if(!bDead){Die();}
	return DamageAmount;
}

bool AVictorCharacter::CanJumpInternal_Implementation() const
{
	if (bHiddenInShadow) { return false; }
	if(!bIsHoldingWall )
	{
		
		// Ensure the character isn't currently crouched.
		bool bCanJump = !bIsCrouched;

		// Ensure that the CharacterMovement state is valid
		bCanJump &= GetCharacterMovement()->CanAttemptJump();

		if (bCanJump)
		{
			// Ensure JumpHoldTime and JumpCount are valid.
			if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
			{
				if (JumpCurrentCount == 0 && GetCharacterMovement()->IsFalling())
				{
					bCanJump = JumpCurrentCount + 1 < JumpMaxCount;
				}
				else
				{
					bCanJump = JumpCurrentCount < JumpMaxCount;
				}
			}
			else
			{
				// Only consider JumpKeyHoldTime as long as:
				// A) The jump limit hasn't been met OR
				// B) The jump limit has been met AND we were already jumping
				const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
				bCanJump = bJumpKeyHeld &&
                            ((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
			}
		}

		return bCanJump;
	}
	else
	{
		return true;
	}
	
}


void AVictorCharacter::OnWallGrabBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{	
	if (GetCharacterMovement() != nullptr)
	{
		if(!GetCharacterMovement()->IsMovingOnGround())
		{
			GetCharacterMovement()->GravityScale = 0.f;
			GetCharacterMovement()->Velocity = FVector(0,0,0);
			GetCharacterMovement()->StopActiveMovement();
			bIsHoldingWall = true;
		}
	}
}

void AVictorCharacter::OnWallGrabBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetCharacterMovement() != nullptr)
	{
		GetCharacterMovement()->GravityScale = 2.f;
		bIsHoldingWall = false;
	}
}

void AVictorCharacter::MoveRight(float Value)
{
	/*UpdateChar();*/

	if(!bPlayingMeleeAttackAnim && !bHiddenInShadow)
	{
		// Apply the input to the character motion
		AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
	}
}

void AVictorCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Jump on any touch
	Jump();
}

void AVictorCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Cease jumping once touch stopped
	StopJumping();
}

void AVictorCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	const FVector PlayerVelocity = GetVelocity();	
	float TravelDirection = PlayerVelocity.X;
	if(!bIsHoldingWall || !bControlledByPlayer)
	{
		// Set the rotation so that the character faces his direction of travel.
		if (Controller != nullptr)
		{
			if (TravelDirection < 0.0f)
			{
				Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
				if(Weapon != nullptr)
				{
					if(Weapon->GetActorLocation().Y!=GetWeaponSocketLocation().Y+0.02)
					{
						Weapon->SetActorLocation(FVector(GetWeaponSocketLocation().X,GetWeaponSocketLocation().Y + 0.02, GetWeaponSocketLocation().Z));						
					}
				}
			}
			else if (TravelDirection > 0.0f)
			{
				Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
				if(Weapon != nullptr)
				{
					if(Weapon->GetActorLocation().Y!=GetSprite()->GetSocketLocation(GetWeaponAttachmentSocketName(Weapon->AnimType)).Y)
					{
						Weapon->SetActorLocation(GetWeaponSocketLocation());						
					}
				}
			}
		}
	}
}
