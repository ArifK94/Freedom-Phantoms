#include "Characters/BaseCharacter.h"
#include "FreedomFighters/FreedomFighters.h"
#include "Weapons/Weapon.h"
#include "Weapons/WeaponBullet.h"
#include "Vehicles/VehicleBase.h"
#include "Visuals/OrderIcon.h"


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
	AimCameraSpring->SetupAttachment(GetMesh());
	AimCameraSpring->bUsePawnControlRotation = true;
	AimCameraSpring->TargetArmLength = 0.0f;
	AimCameraSpring->SocketOffset.Set(0.0f, 0.0f, 0.0f);
	AimCameraSpring->bEnableCameraLag = false;
	AimCameraSpring->bEnableCameraRotationLag = false;

	FirstPersonCameraSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("FirstPersonCameraSpring"));
	FirstPersonCameraSpring->bUsePawnControlRotation = true;
	FirstPersonCameraSpring->SetupAttachment(GetMesh());
	FirstPersonCameraSpring->TargetArmLength = 0.f;
	FirstPersonCameraSpring->SocketOffset.Set(0.f, 0.f, 0.f);
	FirstPersonCameraSpring->bEnableCameraLag = false;
	FirstPersonCameraSpring->bEnableCameraRotationLag = false;


	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetHiddenInGame(false);

	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
	VoiceAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CharacterOutlinePPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("CharacterOutlinePPComp"));
	CharacterOutlinePPComp->SetupAttachment(RootComponent);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	CharacterSpeed = 0.0f;
	CurrentDeltaTime = 0.0f;

	AimCameraFOV = 50.0f;
	AimCameraZoomSpeed = 20.0f;
	CoverDistance = 150.0f;

	DestroyDelayTime = 10.f;

	IsFirstPersonView = false;
	IsSprintDefault = true;
	isSprinting = false;
	UseRootMotion = false;
	isFacingCoverRHS = false;

	HeadSocket = "j_head";
	RightHandSocket = "j_wrist_ri";
	ShoulderRightSocket = "j_shoulder_ri";

	CoverRotationLeftPitch = FVector2D(-10.0f, 10.0f);
	CoverRotationLeftYaw = FVector2D(-20.0f, 0.0f);
	CoverRotationRightPitch = FVector2D(-10.0f, 10.0f);
	CoverRotationRightYaw = FVector2D(0.0f, 20.0f);
}

void ABaseCharacter::SetVehicleSeat(FVehicletSeating Seat)
{
	if (Seat.OwningVehicle)
	{
		CurrentVehicleSeat = Seat;

		if (UseAimCameraSpring)
		{
			FollowCamera->AttachToComponent(Seat.OwningVehicle->GetMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Seat.SeatingSocketName);
			UpdateAimCamera();
		}

		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;

		IsInVehicle = true;
	}
	else
	{
		if (UseAimCameraSpring)
		{
			FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;
		GetWorldTimerManager().SetTimer(THandler_CharacterMovement, this, &ABaseCharacter::UpdateCharacterMovement, .1f, true);
		IsInVehicle = false;
	}

}

void ABaseCharacter::SetIsExitingVehicle(bool IsExiting)
{
	IsExitingVehicle = IsExiting;

	if (IsExiting)
	{
		EndAim();

		if (UseAimCameraSpring)
		{
			FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
	}

	OnRappelUpdate.Broadcast(this);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	RetrieveVoiceDataSet();
	RetrieveAccessoryDataSet();
	RetrieveDeathAnimDataSet();

	DefaultCapsuleCollisionName = GetCapsuleComponent()->GetCollisionProfileName();
	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SprintSpeed = DefaultMaxWalkSpeed * 2.0f;

	DefaultAIController = Cast<AAIController>(GetController());

	DefaultCamSocketOffset = CameraBoom->SocketOffset;
	DefaultCameraFOV = FollowCamera->FieldOfView;

	AimCameraSpring->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ShoulderRightSocket);
	FirstPersonCameraSpring->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocket);

	// Create Animation Instance Object
	AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;

	InitTimeHandlers();

	SpawnOverheadIcon();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnCapsuleHit);
	HealthComp->OnHealthChanged.AddDynamic(this, &ABaseCharacter::OnHealthUpdate);
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

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	UpdateCameraView();

	AimOffset();
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

FRotator ABaseCharacter::GetViewRotation() const
{
	if (IsInVehicle && FollowCamera)
	{
		return FollowCamera->GetComponentRotation();
	}
	return Super::GetViewRotation();
}

void ABaseCharacter::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!HealthComp->IsAlive())
	{
		VoiceAudioComponent->Stop();

		ClearTimeHandlers();

		ShowCharacterOutline(false, true);

		GetCharacterMovement()->StopMovementImmediately();

		EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(InHealthParameters.HitInfo.PhysMaterial.Get());

		// Play death sound if not recieved a headshot
		if (SurfaceType != SURFACE_HEAD)
		{
			if (VoiceClipsSet->DeathSound != nullptr)
			{
				VoiceAudioComponent->Sound = VoiceClipsSet->DeathSound;
				VoiceAudioComponent->Play();
			}
		}



		PlayDeathAnim(InHealthParameters);

		// In case death anim asset is empty
		if (!DeathAnimationAsset) {
			DeathAnimationAsset = DeathAnimation->Defaults[rand() % DeathAnimation->Defaults.Num()];
		}


		GetWorldTimerManager().SetTimer(THandler_Destroyer, this, &ABaseCharacter::StartDestroy, DestroyDelayTime, false);
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

	if (!HealthComp->IsAlive() && OtherActor != this)
	{
		PostDeath();
	}
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	auto Velocity = UKismetMathLibrary::Abs(GetVelocity().Size());

	auto NormalizedVelo = UKismetMathLibrary::NormalizeToRange(Velocity, 1000.f, 1500.f);

	auto ClampVelo = UKismetMathLibrary::ClampAngle(NormalizedVelo, .0f, 1.f);


	FHealthParameters HealthParameters;
	HealthParameters.Damage = ClampVelo * 100.f;
	HealthParameters.InstigatedBy = GetInstigatorController();
	HealthParameters.HitInfo = Hit;
	HealthComp->OnDamage(HealthParameters);

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

void ABaseCharacter::RetrieveDeathAnimDataSet()
{
	if (DeathAnimDatatable == nullptr) {
		return;
	}

	static const FString ContextString(TEXT("DeathAnim DataSet"));
	DeathAnimation = DeathAnimDatatable->FindRow<FDeathAnimation>("0", ContextString, true);
}

void ABaseCharacter::UpdateCameraView()
{
	if (IsFirstPersonView) {
		return;
	}

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

void ABaseCharacter::ToggleSprint()
{
	IsSprintDefault = !IsSprintDefault;

	// to make easier to go from crouch to stand (user experience purpose)
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
}

void ABaseCharacter::BeginSprint()
{
	if (isTakingCover)
	{
		isTakingCover = false;
	}

	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}

	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	isSprinting = true;
}

void ABaseCharacter::EndSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;

	isSprinting = false;
}

void ABaseCharacter::ToggleCrouch()
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

	FRotator InputRotation = GetControlRotation();
	if (IsInVehicle || (isTakingCover && isAtCoverCorner))
	{
		InputRotation = FollowCamera->GetComponentRotation();
	}

	FRotator Current = UKismetMathLibrary::MakeRotator(x, aimPitch, aimYaw);
	FRotator Target = UKismetMathLibrary::NormalizedDeltaRotator(InputRotation, GetActorRotation());
	FRotator MoveToTarget = FMath::RInterpTo(Current, Target, CurrentDeltaTime, 15.0f);

	UKismetMathLibrary::BreakRotator(MoveToTarget, MoveToTarget.Roll, aimPitch, aimYaw);
}

void ABaseCharacter::UpdateCharacterMovement()
{
	UpdateSpeed();

	// check if character is in the air
	IsCharacterInAir = APawn::GetMovementComponent()->IsFalling();

	if (IsSprintDefault)
	{
		if (IsCharacterMoving() && !GetCharacterMovement()->IsCrouching())
		{
			BeginSprint();
		}
		else
		{
			EndSprint();
		}
	}
}

void ABaseCharacter::UpdateSpeed()
{
	auto TargetSpeed = GetVelocity().Size();
	CharacterSpeed = TargetSpeed;
}

void ABaseCharacter::SpawnOverheadIcon()
{
	if (!OverheadIconClass) {
		return;
	}

	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	OverheadIcon = World->SpawnActor<AOrderIcon>(OverheadIconClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (OverheadIcon) {
		OverheadIcon->HideIcon();
		OverheadIcon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);

		FVector HeadLocation = GetMesh()->GetSocketLocation(GetHeadSocket());
		OverheadIcon->SetActorLocation(HeadLocation);
	}
}

void ABaseCharacter::UpdateDirection()
{
	if (IsSprinting())
	{
		// We want to character to face towards camera's forward direction rather than actor's position,
		// this allows run and shoot to rotate towards camera direction
		auto TargetDirection = FollowCamera->GetComponentRotation() - GetActorRotation();
		auto Direction = TargetDirection.Yaw;
		CharacterDirection = Direction;
	}
	else
	{
		// To be used for blendspaces
		if (AnimInstance)
		{
			CharacterDirection = AnimInstance->CalculateDirection(GetVelocity(), GetActorRotation());
		}
	}
}


bool ABaseCharacter::IsCharacterMoving()
{
	auto Velocity = GetCharacterMovement()->Velocity.Size();

	return Velocity > UKismetMathLibrary::Abs(1.f);
}

void ABaseCharacter::SetFirstPersonView()
{
	IsFirstPersonView = true;
	FollowCamera->AttachToComponent(FirstPersonCameraSpring, FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
}

void ABaseCharacter::ShowCharacterOutline(bool CanShow, bool IgnoreDeath)
{
	if (!HealthComp->IsAlive() && !IgnoreDeath) {
		return;
	}

	SetActorOutline(this, CanShow);
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	ShowActorOutlineRecursive(AttachedActors, CanShow);

}

void ABaseCharacter::ShowActorOutlineRecursive(TArray<AActor*> ParentActor, bool CanShow)
{
	if (ParentActor.Num() <= 0) {
		return;
	}

	for (int i = 0; i < ParentActor.Num(); i++)
	{
		AActor* ChildActor = ParentActor[i];

		SetActorOutline(ChildActor, CanShow);

		// Ignore this parent's component
		if (ChildActor != this) {
			TArray<AActor*> ChildAttachedActors;
			ChildActor->GetAttachedActors(ChildAttachedActors);
			ShowActorOutlineRecursive(ChildAttachedActors, CanShow);
		}
	}
}

void ABaseCharacter::SetActorOutline(AActor* Actor, bool CanShow)
{
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
	for (int32 ComponentIdx = 0; ComponentIdx < SkeletalMeshComponents.Num(); ++ComponentIdx)
	{
		auto currentSkel = Cast<USkeletalMeshComponent>(SkeletalMeshComponents[ComponentIdx]);
		currentSkel->SetRenderCustomDepth(CanShow);
	}

	TArray<UStaticMeshComponent*> StaticComponents;
	Actor->GetComponents<UStaticMeshComponent>(StaticComponents);
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
		StopCover();
	}
	else
	{
		FVector Start = GetActorLocation();
		FVector End = (GetActorForwardVector() * CoverDistance) + Start;

		FHitResult OutHit;
		bool LineTrace = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, COLLISION_COVER);

		if (LineTrace)
		{
			if (OutHit.bBlockingHit)
			{
				StartCover(OutHit);
			}
			else
			{
				StopCover();
			}
		}
		else
		{
			StopCover();
		}
	}
}

void ABaseCharacter::StartCover(FHitResult OutHit)
{
	FVector CoverFirstPos = FVector(
		OutHit.ImpactPoint.X - (GetActorForwardVector().X * 50.0f),
		OutHit.ImpactPoint.Y - (GetActorForwardVector().Y * 50.0f),
		GetActorLocation().Z
	);

	//FLatentActionInfo LatentInfo;
	//LatentInfo.CallbackTarget = this;

	//UKismetSystemLibrary::MoveComponentTo(
	//	GetCapsuleComponent(),
	//	CoverFirstPos,
	//	UKismetMathLibrary::MakeRotFromX(OutHit.Normal),
	//	false,
	//	false,
	//	.2f,
	//	false,
	//	EMoveComponentAction::Type::Move,
	//	LatentInfo
	//);

	//SetActorLocation(CoverFirstPos);

	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	GetCharacterMovement()->SetPlaneConstraintNormal(OutHit.Normal);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	bUseControllerRotationYaw = false;
	isTakingCover = true;
}

void ABaseCharacter::StopCover()
{
	GetCharacterMovement()->SetPlaneConstraintEnabled(false);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	isTakingCover = false;
	isAtCoverCorner = false;

	FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);

}

void ABaseCharacter::CoverMovement(float Value)
{
	FVector WallDirection = GetCharacterMovement()->GetPlaneConstraintNormal() * -1.0f; // get direction towards the cover wall

	FVector RightVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallDirection)) * GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector StartRight = GetActorLocation() + RightVector;
	FVector EndRight = WallDirection * CoverDistance + StartRight;
	FHitResult OutHitRight;
	bool LineTraceRight = GetWorld()->LineTraceSingleByChannel(OutHitRight, StartRight, EndRight, COLLISION_COVER);
	//DrawDebugLine(GetWorld(), StartRight, EndRight, FColor::Blue, false, 1, 0, 1);


	FVector LeftVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(GetCharacterMovement()->GetPlaneConstraintNormal())) * GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector StartLeft = GetActorLocation() + LeftVector;
	FVector EndLeft = WallDirection * CoverDistance + StartLeft;
	FHitResult OutHitLeft;
	bool LineTraceLeft = GetWorld()->LineTraceSingleByChannel(OutHitLeft, StartLeft, EndLeft, COLLISION_COVER);
	//DrawDebugLine(GetWorld(), StartLeft, EndLeft, FColor::Red, false, 1, 0, 1);

	FVector Dir = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotator(0.0f, 0.0f, GetControlRotation().Yaw));
	if (LineTraceLeft && LineTraceRight)
	{
		FVector Start = GetActorLocation();
		FVector End = Start + ((GetCharacterMovement()->GetPlaneConstraintNormal() * -1.0f) * CoverDistance);
		FHitResult OutHit;

		bool LineTrace = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, COLLISION_COVER);

		if (LineTrace)
		{
			//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

			if (OutHit.bBlockingHit)
			{
				GetCharacterMovement()->SetPlaneConstraintNormal(OutHit.Normal);
				AddMovementInput(Dir, Value);

				//FVector CoverFirstPos = FVector(
				//	OutHit.ImpactPoint.X - (GetActorForwardVector().X / 100.0f),
				//	OutHit.ImpactPoint.Y - (GetActorForwardVector().Y / 100.0f),
				//	OutHit.ImpactPoint.Z- (GetActorForwardVector().Z / 100.0f)
				//	);

				//DrawDebugSphere(GetWorld(), CoverFirstPos, 10.0f, 20, FColor::Purple, false, 20.0f, 0, 2);

				//SetActorLocation(CoverFirstPos);
				//SetActorRotation(UKismetMathLibrary::MakeRotFromX(WallDirection));
			}
		}
		isAtCoverCorner = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
	}
	else if (LineTraceLeft) // if reached right corner
	{
		if (Value < 0.0f)
		{
			AddMovementInput(Dir, Value);
		}

		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false; // based on corner animation being used, set to false
		isAtCoverCorner = true;
		isFacingCoverRHS = true;

		SetActorRotation(FRotator(0.0f, -90.0f, 0.0f));
		FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);

	}
	else if (LineTraceRight) // if reached left corner
	{
		if (Value > 0.0f)
		{
			AddMovementInput(Dir, Value);
		}


		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false; // based on corner animation being used, set to false
		isAtCoverCorner = true;
		isFacingCoverRHS = false;

		SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
		FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);
	}
}


void ABaseCharacter::BeginAim()
{
	if (isTakingCover)
	{
		float Distance = 150.0f;
		FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
		FVector End = Start + (GetActorForwardVector() * Distance);

		FHitResult OutHit;
		bool LineTrace = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility);
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 1, 0, 1);

		if (LineTrace)
		{
			isAiming = false;
		}
		else
		{
			isAiming = true;
		}
	}
	else
	{
		isAiming = true;
	}

	if (isAiming && UseAimCameraSpring && !IsInVehicle)
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

	if (UseAimCameraSpring && !IsInVehicle)
	{
		FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepWorldTransform);

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;

		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector::ZeroVector, FRotator::ZeroRotator, true, true, 0.3f, false, EMoveComponentAction::Type::Move, LatentInfo);
	}
}

void ABaseCharacter::UpdateAimCamera()
{
	FollowCamera->SetWorldLocation(GetMesh()->GetSocketLocation(ShoulderRightSocket));
}

void ABaseCharacter::PlayVoiceSound(USoundBase* Sound)
{
	if (!HealthComp->IsAlive() || Sound == nullptr) {
		return;
	}

	VoiceAudioComponent->Sound = Sound;
	VoiceAudioComponent->Play();
}

void ABaseCharacter::PlayDeathAnim(FHealthParameters InHealthParameters)
{
	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(InHealthParameters.HitInfo.PhysMaterial.Get());

	// Use dot product to determine where the character stands based on the impact point.
	// DotProduct > 0.0f Same direction
	// DotProduct == 0.0f Perpendicular direction
	// DotProduct < 0.0f Opposite direction
	float DotProduct = FVector::DotProduct(InHealthParameters.HitInfo.ImpactNormal, GetActorForwardVector());

	bool HasHitFront = false;
	bool HasHitLeft = false;
	bool HasHitRight = false;

	if (DotProduct > 0.5f) // If facing the impact point
	{
		HasHitFront = true;
	}
	else if (DotProduct > 0.0f && DotProduct < 0.5f) // if facing left of impact
	{
		HasHitLeft = true;
	}
	else if (DotProduct < 0.0f && DotProduct > -0.5f) // if facing right of impact
	{
		HasHitRight = true;
	}
	//  else behind impact




	if (InHealthParameters.IsExplosive)
	{
		if (isSprinting)
		{
			if (HasHitLeft)
			{
				DeathAnimationAsset = DeathAnimation->SprintExplosionsRight[rand() % DeathAnimation->SprintExplosionsRight.Num()];
			}
			else if (HasHitRight)
			{
				DeathAnimationAsset = DeathAnimation->SprintExplosionsLeft[rand() % DeathAnimation->SprintExplosionsLeft.Num()];
			}
			else
			{
				DeathAnimationAsset = DeathAnimation->SprintExplosionsFront[rand() % DeathAnimation->SprintExplosionsFront.Num()];
			}
		}
		else
		{
			if (HasHitFront)
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsBack[rand() % DeathAnimation->StandExplosionsBack.Num()];
			}
			else if (HasHitLeft)
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsRight[rand() % DeathAnimation->StandExplosionsRight.Num()];
			}
			else if (HasHitRight)
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsLeft[rand() % DeathAnimation->StandExplosionsLeft.Num()];
			}
			else
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsFront[rand() % DeathAnimation->StandExplosionsFront.Num()];
			}
		}
		return;
	}

	if (InHealthParameters.WeaponCauser && InHealthParameters.WeaponCauser->GetWeaponType() == WeaponType::Shotgun)
	{
		if (SurfaceType == SURFACE_LEGS)
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsLegs[rand() % DeathAnimation->ShotgunHitsLegs.Num()];
			return;
		}
		else if (HasHitFront)
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsFront[rand() % DeathAnimation->ShotgunHitsFront.Num()];
			return;
		}
		else if (HasHitLeft)
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsLeft[rand() % DeathAnimation->ShotgunHitsLeft.Num()];
			return;
		}
		else if (HasHitRight)
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsRight[rand() % DeathAnimation->ShotgunHitsRight.Num()];
			return;
		}
	}

	if (isSprinting)
	{
		int RandomIndex = rand() % DeathAnimation->Sprints.Num();
		DeathAnimationAsset = DeathAnimation->Sprints[RandomIndex];
		return;
	}

	if (GetCharacterMovement()->IsCrouching())
	{
		int RandomIndex = rand() % DeathAnimation->Crouches.Num();
		DeathAnimationAsset = DeathAnimation->Crouches[RandomIndex];
		return;
	}


	switch (SurfaceType)
	{
	case SURFACE_HEAD:
		DeathAnimationAsset = DeathAnimation->Headshots[rand() % DeathAnimation->Headshots.Num()];
		return;
	case SURFACE_FLESHVULNERABLE:
		DeathAnimationAsset = DeathAnimation->Vulernables[rand() % DeathAnimation->Vulernables.Num()];
		return;
	case SURFACE_GROIN:
		DeathAnimationAsset = DeathAnimation->Groins[rand() % DeathAnimation->Groins.Num()];
		return;
	default:
		DeathAnimationAsset = DeathAnimation->Defaults[rand() % DeathAnimation->Defaults.Num()];
		return;
	}
}

void ABaseCharacter::PostDeath()
{
	GetMesh()->SetAnimInstanceClass(NULL);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::StartDestroy()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	DetroyChildActor(AttachedActors);
	Destroy();
}

void ABaseCharacter::DetroyChildActor(TArray<AActor*> ParentActor)
{
	for (int i = 0; i < ParentActor.Num(); i++)
	{
		AActor* ChildActor = ParentActor[i];

		if (ChildActor == this) {
			continue;
		}


		TArray<AActor*> ChildAttachedActors;
		ChildActor->GetAttachedActors(ChildAttachedActors);
		DetroyChildActor(ChildAttachedActors);

		ChildActor->Destroy();
	}
}
