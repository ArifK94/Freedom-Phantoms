#include "Characters/BaseCharacter.h"
#include "FreedomFighters/FreedomFighters.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"

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
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Engine.h"

ABaseCharacter::ABaseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

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

	AimCameraSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("AimCameraSpring"));
	AimCameraSpring->SetupAttachment(RootComponent);
	AimCameraSpring->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	AimCameraSpring->TargetArmLength = 100.0f; // The camera follows at this distance behind the character	
	AimCameraSpring->SocketOffset.Set(0.0f, 40.0f, 50.0f);
	AimCameraSpring->bEnableCameraLag = false;
	AimCameraSpring->bEnableCameraRotationLag = false;


	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));

	VoiceAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CharacterOutlinePPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("CharacterOutlinePPComp"));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	CharacterSpeed = 0.0f;
	CurrentDeltaTime = 0.0f;

	AimCameraFOV = 50.0f;
	AimCameraZoomSpeed = 20.0f;

	isSprinting = false;
	isDead = false;
	ReceeivedInitialDirection = false;
	UseRootMotion = false;
	CoverSelected = false;
	isFacingCoverRHS = false;

	HeadSocket = "j_head";
	RightHandSocket = "j_wrist_ri";
	ShoulderRightSocket = "j_shoulder_ri";

	HealthComp->OnHealthChanged.AddDynamic(this, &ABaseCharacter::OnHealthChanged);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	RetrieveVoiceDataSet();
	RetrieveAccessoryDataSet();

	defaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	CamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	DefaultCamViewYawMin = CamManager->ViewYawMin;
	DefaultCamViewYawMax = CamManager->ViewYawMax;

	DefaultCamSocketOffset = CameraBoom->SocketOffset;
	DefaultCameraFOV = FollowCamera->FieldOfView;

	AimCameraSpring->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, ShoulderRightSocket);

	// Create Animation Instance Object
	AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnCapsuleHit);

	InitTimeHandlers();
}

void ABaseCharacter::InitTimeHandlers()
{
	GetWorldTimerManager().SetTimer(THandler_CharacterMovement, this, &ABaseCharacter::UpdateCharacterMovement, .1f, true);
	GetWorldTimerManager().SetTimer(THandler_CharacterDirection, this, &ABaseCharacter::UpdateDirection, .1f, true);
}

void ABaseCharacter::ClearTimeHandlers()
{
	GetWorldTimerManager().ClearTimer(THandler_CharacterMovement);
	GetWorldTimerManager().ClearTimer(THandler_CharacterDirection);
}

void ABaseCharacter::SetIsInAircraft(bool InAircraft)
{
	IsInAircraft = InAircraft;

	if (InAircraft)
	{
		GetWorldTimerManager().ClearTimer(THandler_CharacterMovement);
	}
	else
	{
		GetWorldTimerManager().SetTimer(THandler_CharacterMovement, this, &ABaseCharacter::UpdateCharacterMovement, .1f, true);
	}
}

void ABaseCharacter::SetIsRepellingDown(bool IsRappelling)
{
	isRepellingDown = IsRappelling;
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	UpdateCameraView();

	AimOffset();
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
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

void ABaseCharacter::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !isDead)
	{
		PrimaryActorTick.bCanEverTick = false;
		ClearTimeHandlers();

		isDead = true;

		ShowCharacterOutline(false);

		GetCharacterMovement()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
	}
}

void ABaseCharacter::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);

	if (Character)
	{
		// ignore character collision when moving
		GetCapsuleComponent()->IgnoreActorWhenMoving(Character, true);
		Character->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
	}
}

void ABaseCharacter::RetrieveVoiceDataSet()
{
	if (VoiceClipsDatatable == nullptr || VoiceSetRows.Num() <= 0) {
		return;
	}

	int RandomRowIndex = rand() % VoiceSetRows.Num();
	FName DataRowName = VoiceSetRows[RandomRowIndex];

	static const FString ContextString(TEXT("Voice Clip Set"));
	VoiceClipsSet = VoiceClipsDatatable->FindRow<FVoiceClipSet>(DataRowName, ContextString, true);
}

void ABaseCharacter::RetrieveAccessoryDataSet()
{
	if (AccessoryDatatable == nullptr) {
		return;
	}

	static const FString ContextString(TEXT("Accessory DataSet"));
	AccessorySet = AccessoryDatatable->FindRow<FAccessorySet>(AccessoryRowName, ContextString, true);
}

void ABaseCharacter::UpdateCameraView()
{
	// Set Camera Fix Position when taking cover
	float TargetX = DefaultCamSocketOffset.X;
	float TargetY = DefaultCamSocketOffset.Y;
	float TargetZ = DefaultCamSocketOffset.Z;
	float Speed = 5.0f;


	if (isTakingCover && isAtCoverCorner)
	{
		TargetX = 0.0f;

		if (isFacingCoverRHS)
		{
			TargetY = 70.0f;
		}
		else
		{
			TargetY = -70.0f;
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
	if (!isSprinting)
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

	FRotator Current = UKismetMathLibrary::MakeRotator(x, aimPitch, aimYaw);
	FRotator Target = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());
	FRotator MoveToTarget = FMath::RInterpTo(Current, Target, CurrentDeltaTime, 15.0f);

	UKismetMathLibrary::BreakRotator(MoveToTarget, MoveToTarget.Roll, aimPitch, aimYaw);
}

void ABaseCharacter::UpdateCharacterMovement()
{
	UpdateSpeed();

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
		FVector Velocity = GetVelocity();

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
			CharacterDirection = (AnimInstance->CalculateDirection(Velocity, FollowCamera->GetComponentRotation())) * -1.0f;
		}
	}
}


void ABaseCharacter::ShowCharacterOutline(bool CanShow)
{
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
	for (int32 ComponentIdx = 0; ComponentIdx < SkeletalMeshComponents.Num(); ++ComponentIdx)
	{
		auto currentSkel = Cast<USkeletalMeshComponent>(SkeletalMeshComponents[ComponentIdx]);
		currentSkel->SetRenderCustomDepth(CanShow);
	}

	TArray<UStaticMeshComponent*> StaticComponents;
	GetComponents<UStaticMeshComponent>(StaticComponents);
	for (int32 ComponentIdx = 0; ComponentIdx < StaticComponents.Num(); ++ComponentIdx)
	{
		auto currentStatic = Cast<UStaticMeshComponent>(StaticComponents[ComponentIdx]);
		currentStatic->SetRenderCustomDepth(CanShow);
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
		CoverStart = GetActorLocation();

		float Distance = 150.0f;
		CoverForwardAxis = UKismetMathLibrary::GetForwardVector(GetActorRotation());

		FVector End = ((CoverForwardAxis * Distance) + CoverStart);

		auto SphereLineTrace = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), CoverStart, End, 50.0f, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true);

		if (SphereLineTrace)
		{
			if (OutHit.bBlockingHit)
			{

				WallLocation = OutHit.ImpactPoint;
				WallNormal = OutHit.ImpactNormal;

				MoveToCover();

				if (isSprinting)
					isSprinting = false;

				isTakingCover = true;
				GetCharacterMovement()->bOrientRotationToMovement = false;
				GetCharacterMovement()->bUseControllerDesiredRotation = false;

				CheckCoverType();
			}
		}
	}
}

void ABaseCharacter::MoveToCover()
{
	FVector CoverFirstPos = FVector(
		WallLocation.X - (CoverForwardAxis.X * 35.0f),
		WallLocation.Y - (CoverForwardAxis.Y * 35.0f),
		CoverStart.Z
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

	bUseControllerRotationYaw = false;
}

void ABaseCharacter::CoverMovement(float Value)
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

	// Right side of cover from camera's perspective which is equal to
	// character's left hand side
	if (Value == 1)
	{
		Start = UKismetMathLibrary::Subtract_VectorVector(ActorLocation, CapsuleRight);
		End = NewRightLocation;
		isFacingCoverRHS = true;
	}
	else
	{
		Start = UKismetMathLibrary::Add_VectorVector(ActorLocation, CapsuleRight);
		End = NewLeftLocation;
		isFacingCoverRHS = false;
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
			CoverStart = Start;
			WallLocation = OutHit.ImpactPoint;

			CoverSelected = true;
			isAtCoverCorner = false;

			CamManager->ViewYawMin = DefaultCamViewYawMin;
			CamManager->ViewYawMax = DefaultCamViewYawMax;

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


		// Clamp the camera view in the Yaw
		if (isFacingCoverRHS)
		{
			//CamManager->ViewYawMin = -90.0f;
			//CamManager->ViewYawMax = 0.0f;
		}
		else
		{
			//CamManager->ViewYawMin = -180.0f;
			//CamManager->ViewYawMax = -90.0f;
		}
	}
}

void ABaseCharacter::CheckCoverType()
{
	// Can Move In Cover
	float CoverDistance = 60.0f;



	FVector ActorLocation = WallLocation;
	FRotator TargetRotation = UKismetMathLibrary::MakeRotFromXZ(UKismetMathLibrary::Multiply_VectorVector(WallNormal, FVector(-1.0f, -1.0f, 0.0f)), FVector(0.0f, 0.0f, WallLocation.Z));

	FVector TargetFoward = UKismetMathLibrary::GetForwardVector(TargetRotation);
	FVector RightDirection = UKismetMathLibrary::GetRightVector(TargetRotation);


	FVector NewForward = UKismetMathLibrary::Add_VectorVector(FVector(TargetFoward.X * 50.0f, TargetFoward.Y * 50.0f, 0.0f), ActorLocation);
	FVector NewRight = FVector(RightDirection.X * CoverDistance, RightDirection.Y * CoverDistance, 0.0f);

	FVector TargetAdd = UKismetMathLibrary::Add_VectorVector(NewForward, NewRight);
	FVector TargetMinus = UKismetMathLibrary::Subtract_VectorVector(NewForward, NewRight);

	FVector NewRightLocation = FVector(TargetAdd.X, TargetAdd.Y, ActorLocation.Z);
	FVector NewLeftLocation = FVector(TargetMinus.X, TargetMinus.Y, ActorLocation.Z);

	FVector StartLeft = FVector(NewLeftLocation.X + 100.0f, NewLeftLocation.Y, NewLeftLocation.Z);
	FVector StartRight = FVector(NewRightLocation.X + 100.0f, NewRightLocation.Y, NewRightLocation.Z);

	DrawDebugDirectionalArrow(GetWorld(), StartLeft, NewLeftLocation, 120.f, FColor::Magenta, true, 30.0f, 0, 5.f);
	DrawDebugDirectionalArrow(GetWorld(), StartRight, NewRightLocation, 120.f, FColor::Red, true, 30.0f, 0, 5.f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	FHitResult OutHitLeft, OutHitRight;
	bool LineTraceLeft = GetWorld()->LineTraceSingleByObjectType(OutHitLeft, StartLeft, NewLeftLocation, ObjectParams, QueryParams);

	bool LineTraceRight = GetWorld()->LineTraceSingleByObjectType(OutHitRight, StartRight, NewRightLocation, ObjectParams, QueryParams);


	if (!LineTraceLeft)	// if no line trace on left side then it is facing the left cover corner
	{
		isAtCoverCorner = true;
		isFacingCoverRHS = false;
		//CamManager->ViewYawMin = -180.0f;
		//CamManager->ViewYawMax = -90.0f;
	}
	else if (!LineTraceRight)
	{
		isAtCoverCorner = true;
		isFacingCoverRHS = true;
		//CamManager->ViewYawMin = -90.0f;
		//CamManager->ViewYawMax = 0.0f;
	}
	else
	{
		if (OutHitLeft.bBlockingHit && OutHitRight.bBlockingHit)
		{
			CoverSelected = true;
			isAtCoverCorner = false;

			CamManager->ViewYawMin = DefaultCamViewYawMin;
			CamManager->ViewYawMax = DefaultCamViewYawMax;
		}
	}
}

void ABaseCharacter::EscapeCover()
{
	isTakingCover = false;
	CoverSelected = false;
	isAtCoverCorner = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	CamManager->ViewYawMin = DefaultCamViewYawMin;
	CamManager->ViewYawMax = DefaultCamViewYawMax;
}

void ABaseCharacter::ResetInitialDirectionBool()
{
	ReceeivedInitialDirection = true;
	GetWorldTimerManager().ClearTimer(THandler_ResetInitialDirectionBool);

}

void ABaseCharacter::BeginAim()
{
	isAiming = true;

	if (UseAimCameraSpring)
	{
		FollowCamera->AttachToComponent(AimCameraSpring, FAttachmentTransformRules::KeepWorldTransform);

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector::ZeroVector, FRotator::ZeroRotator, true, true, 0.3f, false, EMoveComponentAction::Type::Move, LatentInfo);
	}
}

void ABaseCharacter::EndAim()
{
	isAiming = false;

	if (UseAimCameraSpring)
	{
		FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepWorldTransform);

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;

		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector::ZeroVector, FRotator::ZeroRotator, true, true, 0.3f, false, EMoveComponentAction::Type::Move, LatentInfo);
	}
}

