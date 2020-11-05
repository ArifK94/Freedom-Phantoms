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
	CameraBoom->TargetArmLength = 200.0f; // The camera follows at this distance behind the character	
	CameraBoom->SocketOffset.Set(0.0f, 40.0f, 50.0f);

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 50.0f;
	CameraBoom->CameraRotationLagSpeed = 50.0f;
	CameraBoom->CameraLagMaxDistance = 10.0f;


	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));

	VoiceAudioComponent->AttachTo(RootComponent);

	CharacterOutlinePPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("CharacterOutlinePPComp"));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	CharacterSpeed = 0.0f;
	CurrentDeltaTime = 0.0f;

	AimCameraFOV = 50.0f;
	AimCameraZoomSpeed = 20.0f;

	isSprinting = false;
	isDead = false;
	canMoveForward = false;
	ReceeivedInitialDirection = false;
	UseRootMotion = false;
	CoverSelected = false;
	isFacingLeftCover = false;

	HealthComp->OnHealthChanged.AddDynamic(this, &ABaseCharacter::OnHealthChanged);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultCamSocketOffset = CameraBoom->SocketOffset;
	DefaultCameraFOV = FollowCamera->FieldOfView;

	canMoveForward = true;

	// Create Animation Instance Object
	AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	UpdateCameraView();

	UpdateCharacterMovement();

	AimOffset();

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

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABaseCharacter::BeginAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABaseCharacter::EndAim);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABaseCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABaseCharacter::EndSprint);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABaseCharacter::BeginCrouch);

	PlayerInputComponent->BindAction("TakeCover", IE_Pressed, this, &ABaseCharacter::TakeCover);

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
	if (Controller != NULL)
	{
		ForwardInputValue = Value;

		if (Value < 0.0f || canMoveForward)
		{
			if (Value != 0.0f)
			{
				if (!isTakingCover)
				{
					// find out which way is forward
					const FRotator Rotation = Controller->GetControlRotation();
					const FRotator YawRotation(0, Rotation.Yaw, 0);

					// get forward vector
					const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

					AddMovementInput(Direction, Value);
				}
				else
				{
					if (Value == -1.0f)
					{
						EscapeCover();
					}
				}
			}
		}
	}


}

void ABaseCharacter::MoveRight(float Value)
{
	if (Controller != NULL)
	{
		if (!isTakingCover || isAtCoverCorner)
			RightInputValue = Value;

		if (Value != 0.0f)
		{
			if (isTakingCover && !isAtCoverCorner)
				RightInputValue = Value;

			if (!isTakingCover)
			{
				//find out which way is right
				const FRotator Rotation = Controller->GetControlRotation();
				const FRotator YawRotation(0, Rotation.Yaw, 0);

				CoverSelected = false;

				// get right vector 
				const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
				// add movement in that direction
				AddMovementInput(Direction, Value);
			}
			else
			{
				// Can Move In Cover
				float CoverDistance = 40.0f;

				FVector ActorLocation = GetActorLocation();
				FVector Start, End;
				FRotator TargetRotation = UKismetMathLibrary::MakeRotFromXZ(UKismetMathLibrary::Multiply_VectorVector(WallNormal, FVector(-1.0f, -1.0f, 0.0f)), GetCapsuleComponent()->GetUpVector());

				FVector TargetFoward = UKismetMathLibrary::GetForwardVector(TargetRotation);
				FVector RightDirection = UKismetMathLibrary::GetRightVector(TargetRotation);


				FVector NewForward = UKismetMathLibrary::Add_VectorVector(FVector(TargetFoward.X * 50.0f, TargetFoward.Y * 50.0f, 0.0f), ActorLocation);
				FVector NewRight = FVector(RightDirection.X * CoverDistance, RightDirection.Y * CoverDistance, 0.0f);

				FVector TargetAdd = UKismetMathLibrary::Add_VectorVector(NewForward, NewRight);
				FVector TargetMinus = UKismetMathLibrary::Subtract_VectorVector(NewForward, NewRight);

				FVector NewRightLocation = FVector(TargetAdd.X, TargetAdd.Y, ActorLocation.Z);
				FVector NewLeftLocation = FVector(TargetMinus.X, TargetMinus.Y, ActorLocation.Z);




				FVector CapsuleRight = GetCapsuleComponent()->GetRightVector() * CoverDistance;

				if (Value == 1)
				{
					Start = UKismetMathLibrary::Subtract_VectorVector(ActorLocation, CapsuleRight);
					End = NewRightLocation;
					isFacingLeftCover = true;
				}
				else
				{
					Start = UKismetMathLibrary::Add_VectorVector(ActorLocation, CapsuleRight);
					End = NewLeftLocation;
					isFacingLeftCover = false;
				}

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(this);
				QueryParams.bTraceComplex = false;
				QueryParams.bReturnPhysicalMaterial = true;

				FCollisionObjectQueryParams ObjectParams;
				ObjectParams.AllObjects;

				DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

				FHitResult OutHit;
				auto LineTrace = GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, QueryParams);


				if (LineTrace)
				{
					if (OutHit.bBlockingHit)
					{
						CoverSelected = true;
						isAtCoverCorner = false;


						//find out which way is right
						const FRotator Rotation = Controller->GetControlRotation();
						const FRotator YawRotation(0, Rotation.Yaw, 0);

						// add movement in that direction
						AddMovementInput(RightDirection, Value);
					}
				}
				else
				{
					isAtCoverCorner = true;
				}

			}
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

void ABaseCharacter::UpdateCameraView()
{

	// Camera Direction
	float TargetX = DefaultCamSocketOffset.X;
	float TargetY = DefaultCamSocketOffset.Y;
	float TargetZ = DefaultCamSocketOffset.Z;
	float Speed = 5.0f;

	if (isTakingCover && isAtCoverCorner)
	{
		TargetX = 0.0f;
		if (!isFacingLeftCover)
		{
			TargetY = -70.0f;
		}
		else
		{
			TargetY = 70.0f;
		}

		TargetZ = 50.0f;
	}

	float XInterp = FMath::FInterpTo(CameraBoom->SocketOffset.X, TargetX, CurrentDeltaTime, Speed);
	float YInterp = FMath::FInterpTo(CameraBoom->SocketOffset.Y, TargetY, CurrentDeltaTime, Speed);
	float ZInterp = FMath::FInterpTo(CameraBoom->SocketOffset.Z, TargetZ, CurrentDeltaTime, Speed);

	CameraBoom->SocketOffset.Set(XInterp, YInterp, ZInterp);


	// Camera Zooming
	float TargetFOV = isAiming ? AimCameraFOV : DefaultCameraFOV;
	float ZoomInterp = FMath::FInterpTo(FollowCamera->FieldOfView, TargetFOV, CurrentDeltaTime, AimCameraZoomSpeed);
	FollowCamera->SetFieldOfView(ZoomInterp);
	

}

void ABaseCharacter::BeginSprint()
{
	if (CharacterSpeed > 0.1f)
	{
		if (isTakingCover)
		{
			isTakingCover = false;
		}

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
	UpdateSpeed();
	UpdateDirection();

	// check if character is in the air
	IsCharacterInAir = APawn::GetMovementComponent()->IsFalling();
}

void ABaseCharacter::UpdateSpeed()
{
	float TargetSpeed = 0.0f;

	if (UseRootMotion)
	{
		TargetSpeed = FMath::Clamp(FMath::Abs(ForwardInputValue) + FMath::Abs(RightInputValue), 0.0f, 1.0f);

		if (isSprinting)
		{
			TargetSpeed = TargetSpeed * 2.0f;
		}

		if (LastForwardInputVal != ForwardInputValue || LastRightInput != RightInputValue)
		{
			ChangedCharacterDirection = true;
		}
		else
		{
			ChangedCharacterDirection = false;
		}

		LastForwardInputVal = ForwardInputValue;
		LastRightInput = RightInputValue;
	}
	else
	{
		FVector Velocity = AActor::GetVelocity();

		TargetSpeed = Velocity.Size();

		if (isSprinting)
		{
			GetCharacterMovement()->MaxWalkSpeed = defaultMaxWalkSpeed * 2.0f;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = defaultMaxWalkSpeed;
		}
	}
	CharacterSpeed = TargetSpeed;



}

void ABaseCharacter::UpdateDirection()
{
	if (UseRootMotion)
	{
		float x = 0.0f, y = 0.0f;
		float Pitch = 0.0f, Yaw = 0.0f, Roll = 0.0f;

		FVector InputPos = UKismetMathLibrary::MakeVector(ForwardInputValue, RightInputValue, 0.0f);

		FRotator rotation = UKismetMathLibrary::MakeRotFromX(InputPos);

		FRotator Rotator = UKismetMathLibrary::NormalizedDeltaRotator(FollowCamera->GetComponentRotation(), GetCapsuleComponent()->GetComponentRotation());
		FRotator TargetRotator = UKismetMathLibrary::NormalizedDeltaRotator(rotation, Rotator);


		if (CharacterSpeed > 0.01f)
		{
			if (ReceeivedInitialDirection)
			{
				if (CharacterSpeed < 0.01f)
				{
					ReceeivedInitialDirection = false;
				}
			}
			else
			{
				CharacterDirection = TargetRotator.Yaw;
				GetWorldTimerManager().SetTimer(THandler_ResetInitialDirectionBool, this, &ABaseCharacter::ResetInitialDirectionBool, 1.0f, false, .1f);
			}
		}
		else
		{
			if (CharacterSpeed < 0.01f)
			{
				ReceeivedInitialDirection = false;
			}
		}
	}
	else
	{
		FVector Velocity = AActor::GetVelocity();

		// get the direction of the character
		if (AnimInstance)
		{
			CharacterDirection = AnimInstance->CalculateDirection(Velocity, GetActorRotation());
		}
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
		EscapeCover();
	}
	else
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = false;

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AllObjects;

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
		ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2);
		ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery_MAX);
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

		TArray<AActor*> ActorsToIgnore;

		FHitResult OutHit;
		FVector Start = GetActorLocation();

		float Distance = 150.0f;
		FVector ForwardVector = UKismetMathLibrary::GetForwardVector(GetActorRotation());

		FVector End = ((ForwardVector * Distance) + Start);

		auto SphereLineTrace = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), Start, End, 50.0f, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, true);

		if (SphereLineTrace)
		{
			if (OutHit.bBlockingHit)
			{

				WallLocation = OutHit.ImpactPoint;
				WallNormal = OutHit.ImpactNormal;

				FVector CoverFirstPos = FVector(
					WallLocation.X - (ForwardVector.X * 35.0f),
					WallLocation.Y - (ForwardVector.Y * 35.0f),
					Start.Z
				);

				FLatentActionInfo LatentInfo;
				LatentInfo.CallbackTarget = this;

				UKismetSystemLibrary::MoveComponentTo(
					GetCapsuleComponent(),
					CoverFirstPos,
					UKismetMathLibrary::MakeRotFromXZ(WallNormal, GetCapsuleComponent()->GetUpVector()),
					false,
					false,
					.2f,
					false,
					EMoveComponentAction::Type::Move,
					LatentInfo
				);

				if (isSprinting)
					isSprinting = false;

				isTakingCover = true;
				bUseControllerRotationYaw = false;
				GetCharacterMovement()->bOrientRotationToMovement = false;
				GetCharacterMovement()->bUseControllerDesiredRotation = false;
			}
		}

	}

}

void ABaseCharacter::EscapeCover()
{
	isTakingCover = false;
	CoverSelected = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

bool ABaseCharacter::IsFacingCoverAngle()
{
	float differenceAngle = FVector::DotProduct(UKismetMathLibrary::GetForwardVector(GetActorRotation()), UKismetMathLibrary::GetForwardVector(CoverRotation));

	float facingAngle = UKismetMathLibrary::DegAcos(differenceAngle);

	if (facingAngle < 55.0f)
		return true;

	return false;
}


void ABaseCharacter::RenableMovementInput()
{
	Controller->SetIgnoreMoveInput(false);
	GetWorldTimerManager().ClearTimer(THandler_MovemntInputDisable);
}

void ABaseCharacter::ResetInitialDirectionBool()
{
	ReceeivedInitialDirection = true;
	GetWorldTimerManager().ClearTimer(THandler_ResetInitialDirectionBool);

}

void ABaseCharacter::BeginAim()
{
	isAiming = true;
}

void ABaseCharacter::EndAim()
{
	isAiming = false;
}

