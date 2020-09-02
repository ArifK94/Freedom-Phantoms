// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"

#include "Props/BaseCoverProp.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SplineComponent.h"
#include "Components/PostProcessComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Animation/AnimInstance.h"

#include "Kismet/KismetMathLibrary.h"
#include "Engine.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	//GetCapsuleComponent()->SetCollisionResponseToChannels(COLLISION_WEAPON, ECR_Ignore);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 150.0f;
	defaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->TargetArmLength = 150.0f; // The camera follows at this distance behind the character	
	CameraBoom->SocketOffset.Set(0.0f, 0.0f, 0.0f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));

	CharacterOutlinePPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("CharacterOutlinePPComp"));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	CharacterSpeed = 0.0f;
	CurrentDeltaTime = 0.0f;

	isSprinting = false;
	isDead = false;
	canMoveForward = false;

}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultCamSocketOffset = CameraBoom->SocketOffset;

	canMoveForward = true;

	// Create Anim Instance Object
	AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseCharacter::OnCharacterBeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ABaseCharacter::OnCharacterEndOverlap);

	HealthComp->OnHealthChanged.AddDynamic(this, &ABaseCharacter::OnHealthChanged);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	UpdateCameraView();

	UpdateCharacterMovement();

	AimOffset();

	UpdateSprint();

	UpdateCover();

	if (isDead)
	{
		ShowCharacterOutline(false);
	}
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABaseCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABaseCharacter::EndSprint);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABaseCharacter::BeginCrouch);

	PlayerInputComponent->BindAction("TakeCover", IE_Pressed, this, &ABaseCharacter::TakeCover);

	PlayerInputComponent->BindAction("PeakUp", IE_Pressed, this, &ABaseCharacter::BeginPeakAround);
	PlayerInputComponent->BindAction("PeakUp", IE_Released, this, &ABaseCharacter::EndPeakAround);

	PlayerInputComponent->BindAction("PeakLeft", IE_Pressed, this, &ABaseCharacter::BeginPeakAround);
	PlayerInputComponent->BindAction("PeakLeft", IE_Released, this, &ABaseCharacter::EndPeakAround);

	PlayerInputComponent->BindAction("PeakDown", IE_Pressed, this, &ABaseCharacter::BeginPeakAround);
	PlayerInputComponent->BindAction("PeakDown", IE_Released, this, &ABaseCharacter::EndPeakAround);

	PlayerInputComponent->BindAction("PeakRight", IE_Pressed, this, &ABaseCharacter::BeginPeakAround);
	PlayerInputComponent->BindAction("PeakRight", IE_Released, this, &ABaseCharacter::EndPeakAround);


	PlayerInputComponent->BindAxis("MoveForward", this, &ABaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turn rate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABaseCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABaseCharacter::LookUpAtRate);
}

// Firing from the center of camera
FVector ABaseCharacter::GetPawnViewLocation() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void ABaseCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABaseCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void ABaseCharacter::MoveForward(float Value)
{
	//Controller->SetIgnoreMoveInput(false);

	if (Value < 0.0f || canMoveForward)
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			EndPeakAround();

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			AddMovementInput(Direction, Value);
		}
	}

	if (isTakingCover)
	{
		if (CurrentCoverObj)
		{
			if (Value > 0.0f && CurrentCoverObj->getCanPeakUp())
				PeakDirection = FVector(0.0F, 0.0F, Value);
			else if (Value < 0.0f)
				PeakDirection = FVector(0.0F, 0.0F, -Value);
		}

	}

}

void ABaseCharacter::MoveRight(float Value)
{
//	Controller->SetIgnoreMoveInput(false);

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		EndPeakAround();

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}

	if (isTakingCover)
	{
		if (CharacterSpeed == 0.0f)
		{
			if (Value > 0.0f)
				PeakDirection = FVector(0.0F, Value, 0.0f);
			else if (Value < 0.0f)
				PeakDirection = FVector(0.0F, Value, 0.0f);
		}

	}

}

void ABaseCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !isDead)
	{
		isDead = true;
		GetCharacterMovement()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
	}
}

void ABaseCharacter::OnCharacterBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		// Check if we have overlapped with a cover actor
		if (OtherActor->GetClass()->IsChildOf(ABaseCoverProp::StaticClass()))
		{
			// cast this cover actor to get the object
			CurrentCoverObj = Cast<ABaseCoverProp>(OtherActor);

			if (CurrentCoverObj)
			{
				canTakeCover = true;
				CoverRotation = CurrentCoverObj->getArrowDirection()->GetComponentRotation();
				isAtCoverCorner = CurrentCoverObj->getIsCorner();

				CurrentCoverType = CurrentCoverObj->getCornerType();
			}
		}
	}
}

void ABaseCharacter::OnCharacterEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		if (OtherActor->GetClass()->IsChildOf(ABaseCoverProp::StaticClass()))
		{
			// cast this cover actor to get the object
			ABaseCoverProp* PreviousCoverObj = Cast<ABaseCoverProp>(OtherActor);

			// Check if last cover object is the same as the one just entered?
			// if so, then get out of cover
			// this is to prevent from coming out of cover if another cover box entered and the current has ended overlap
			if (CurrentCoverObj == PreviousCoverObj)
			{
				canTakeCover = false;
				isTakingCover = false;
			}

		}
	}
}

void ABaseCharacter::UpdateCameraView()
{
	if (isTakingCover)
	{
		if (CurrentCoverType == CoverCornerType::Left)
		{
			CameraBoom->SocketOffset.Set(0.0f, -70.0f, 50.0f);
		}
		else if (CurrentCoverType == CoverCornerType::Right)
		{
			CameraBoom->SocketOffset.Set(0.0f, 70.0f, 50.0f);
		}
	}
	else
	{
		if (CameraBoom->SocketOffset != DefaultCamSocketOffset)
			CameraBoom->SocketOffset = DefaultCamSocketOffset;
	}
}


void ABaseCharacter::UpdateSprint()
{
	if (isSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = defaultMaxWalkSpeed * 2.5f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = defaultMaxWalkSpeed;
	}
}

void ABaseCharacter::BeginSprint()
{
	if (CharacterSpeed > 0.0f)
	{
		isSprinting = true;

		if (GetCharacterMovement()->IsCrouching())
		{
			UnCrouch();
		}

	}
}

void ABaseCharacter::EndSprint()
{
	isSprinting = false;
}

void ABaseCharacter::BeginCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABaseCharacter::AimOffset()
{
	float x = 0.0f;

	FRotator ControlRotation = GetControlRotation();
	FRotator ActorRotation = GetActorRotation();

	FRotator Current = UKismetMathLibrary::MakeRotator(x, aimPitch, aimYaw);
	FRotator Target = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
	FRotator MoveToTarget = FMath::RInterpTo(Current, Target, CurrentDeltaTime, 15.0f);

	UKismetMathLibrary::BreakRotator(MoveToTarget, MoveToTarget.Roll, aimPitch, aimYaw);
}

void ABaseCharacter::UpdateCharacterMovement()
{
	FVector Velocity = AActor::GetVelocity();

	// get the speed of the character
	CharacterSpeed = Velocity.Size();

	// get the direction of the character
	if (AnimInstance)
	{
		CharacterDirection = AnimInstance->CalculateDirection(Velocity, GetActorRotation());
	}

	// check if character is in the air
	IsCharacterInAir = APawn::GetMovementComponent()->IsFalling();
}

void ABaseCharacter::BeginPeakAround()
{
	if (isTakingCover)
	{
		isPeakingAround = true;
		canMoveForward = false;
	}
}

void ABaseCharacter::EndPeakAround()
{
	if (isTakingCover)
	{
		isPeakingAround = false;
		canMoveForward = true;
	}

}

void ABaseCharacter::ShowCharacterOutline(bool CanShow)
{
	TArray<UActorComponent*> SkeletalMeshComponents = this->GetComponentsByClass(USkeletalMeshComponent::StaticClass());
	for (int32 ComponentIdx = 0; ComponentIdx < SkeletalMeshComponents.Num(); ++ComponentIdx)
	{
		auto currentSkel = Cast<USkeletalMeshComponent>(SkeletalMeshComponents[ComponentIdx]);
		currentSkel->SetRenderCustomDepth(CanShow);
	}

	TArray<UActorComponent*> StaticComponents = this->GetComponentsByClass(UStaticMeshComponent::StaticClass());
	for (int32 ComponentIdx = 0; ComponentIdx < StaticComponents.Num(); ++ComponentIdx)
	{
		auto currentSkel = Cast<UStaticMeshComponent>(StaticComponents[ComponentIdx]);
		currentSkel->SetRenderCustomDepth(CanShow);
	}
}



void ABaseCharacter::TakeCover()
{
	if (isTakingCover)
	{
		isTakingCover = false;
	}
	else
	{
		if (canTakeCover)
		{
			isTakingCover = true;
		}
		else
		{
			isTakingCover = false;
		}
	}

}

bool ABaseCharacter::IsFacingCoverAngle()
{
	float differenceAngle = FVector::DotProduct(UKismetMathLibrary::GetForwardVector(GetActorRotation()), UKismetMathLibrary::GetForwardVector(CoverRotation));

	float facingAngle = UKismetMathLibrary::DegAcos(differenceAngle);

	if (facingAngle < 55.0f)
		return true;

	return false;
}

void ABaseCharacter::UpdateCover()
{
	if (isTakingCover)
	{
		SetActorRotation(CoverRotation.Quaternion());

		//if (CurrentCoverType != CoverCornerType::None)
		//{
		//	Controller->SetIgnoreMoveInput(true);
		//	GetWorldTimerManager().SetTimer(THandler_MovemntInputDisable, this, &ABaseCharacter::RenableMovementInput, 5.0f, true, 0.0f);
		//}
	}

	if (FVector::DotProduct(UKismetMathLibrary::GetForwardVector(GetControlRotation()), UKismetMathLibrary::GetForwardVector(CoverRotation)) > 0.9f && isTakingCover)
	{
		canMoveForward = false;
	}
	else
	{
		canMoveForward = true;
	}
}

void ABaseCharacter::RenableMovementInput()
{
	Controller->SetIgnoreMoveInput(false);
	GetWorldTimerManager().ClearTimer(THandler_MovemntInputDisable);

}

