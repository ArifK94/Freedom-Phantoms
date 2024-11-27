#include "Characters/BaseCharacter.h"
#include "FreedomPhantoms/FreedomPhantoms.h"
#include "Weapons/Weapon.h"
#include "Weapons/Projectile.h"
#include "Vehicles/VehicleBase.h"
#include "Visuals/OrderIcon.h"
#include "Managers/GameModeManager.h"
#include "Managers/DatatableManager.h"
#include "Services/SharedService.h"
#include "CustomComponents/RappellerComponent.h"
#include "CustomComponents/OptimizerComponent.h"

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
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Engine.h"
#include "AIController.h"


ABaseCharacter::ABaseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

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
	GetCharacterMovement()->PrimaryComponentTick.TickInterval = 0.03f; // 30ms tick
	GetCharacterMovement()->bEnablePhysicsInteraction = false;
	GetCharacterMovement()->MaxSimulationIterations = 4;


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

	AimCameraRightSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("AimCameraRightSpring"));
	AimCameraRightSpring->SetupAttachment(GetMesh());
	AimCameraRightSpring->bUsePawnControlRotation = true;
	AimCameraRightSpring->TargetArmLength = 100.f;
	AimCameraRightSpring->SocketOffset.Set(0.0f, 45.f, 0.0f);
	AimCameraRightSpring->bEnableCameraLag = false;
	AimCameraRightSpring->bEnableCameraRotationLag = false;


	AimCameraLeftSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("AimCameraLeftSpring"));
	AimCameraLeftSpring->SetupAttachment(GetMesh());
	AimCameraLeftSpring->bUsePawnControlRotation = true;
	AimCameraLeftSpring->TargetArmLength = 100.f;
	AimCameraLeftSpring->SocketOffset.Set(0.0f, -45.f, 0.0f);
	AimCameraLeftSpring->bEnableCameraLag = false;
	AimCameraLeftSpring->bEnableCameraRotationLag = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetHiddenInGame(false);

	VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
	VoiceAudioComponent->bAutoActivate = false;
	VoiceAudioComponent->SetComponentTickEnabled(false);
	VoiceAudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CharacterOutlinePPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("CharacterOutlinePPComp"));
	CharacterOutlinePPComp->SetupAttachment(RootComponent);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	OptimizerComponent = CreateDefaultSubobject<UOptimizerComponent>(TEXT("OptimizerComponent"));
	RappellerComponent = CreateDefaultSubobject<URappellerComponent>(TEXT("RappellerComponent"));

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
	ShoulderLeftocket = "j_shoulder_le";
	ShoulderRightSocket = "j_shoulder_ri";
	LeftFootSocket = "j_ball_le";
	RightFootSocket = "j_ball_ri";

	FootRowName = "Foot";

	CoverPeakUpOffset = FVector::ZeroVector;
	CoverRotationLeftPitch = FVector2D(-10.0f, 10.0f);
	CoverRotationLeftYaw = FVector2D(-20.0f, 0.0f);
	CoverRotationRightPitch = FVector2D(-10.0f, 10.0f);
	CoverRotationRightYaw = FVector2D(0.0f, 20.0f);
	CoverStartYawOffset = .0f;
	CoverMovementYawOffset = .0f;
	CoverCornerLeftYawOffset = .0f;
	CoverCornerRightYawOffset = .0f;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	GameModeManager = Cast<AGameModeManager>(GetWorld()->GetAuthGameMode());

	FootSurfaceImpact = UDatatableManager::RetrieveSurfaceImpact(GetWorld(), FootRowName);

	RetrieveVoiceDataSet();
	RetrieveAccessoryDataSet();
	RetrieveDeathAnimDataSet();

	DefaultCapsuleCollisionName = GetCapsuleComponent()->GetCollisionProfileName();
	DefaultMeshCollisionName = GetMesh()->GetCollisionProfileName();
	DefaultController = GetController();
	DefaultMeshLocation = GetMesh()->GetRelativeLocation();
	DefaultMeshRotation = GetMesh()->GetRelativeRotation();

	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SprintSpeed = DefaultMaxWalkSpeed * 2.0f;

	DefaultAIController = Cast<AAIController>(GetController());

	DefaultCamSocketOffset = CameraBoom->SocketOffset;
	DefaultCameraFOV = FollowCamera->FieldOfView;

	AimCameraLeftSpring->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ShoulderLeftocket);
	AimCameraRightSpring->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ShoulderRightSocket);

	InitTimeHandlers();

	SpawnOverheadIcon();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnCapsuleHit);
	HealthComp->OnHealthChanged.AddDynamic(this, &ABaseCharacter::OnHealthUpdate);
	RappellerComponent->OnRappelChanged.AddDynamic(this, &ABaseCharacter::OnRappelChange);

	GetWorld()->GetTimerManager().SetTimer(THandler_DelayedBeginPlay, this, &ABaseCharacter::BeginDelayedPlay, 1.f, false, 1.f);
}

void ABaseCharacter::BeginDelayedPlay()
{
	if (!IsPlayerControlled())
	{
		DestroyUnusedComponents();
	}

	if (RappellerComponent && !IsInVehicle) RappellerComponent->DestroyComponent();

	GetWorld()->GetTimerManager().ClearTimer(THandler_DelayedBeginPlay);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	UpdateCameraView();

	AimOffset();
}

void ABaseCharacter::Revived()
{
	InitTimeHandlers();
	DefaultController->Possess(this);
}

void ABaseCharacter::Init()
{
	isSprinting = false;
	EndAim();
}

void ABaseCharacter::SetDefaultState()
{
	SetVehicleSeat(FVehicletSeating());
	EndSprint();
	EndAim();
	UnCrouch();
}

void ABaseCharacter::SetVehicleSeat(FVehicletSeating Seat)
{
	bool RemoveRappelComponent = false;
	if (Seat.OwningVehicle)
	{
		CurrentVehicleSeat = Seat;

		// if character will not exit the vehicle, then remove the rappel component.
		if (!Seat.ExitPassengerOnPoint)
		{
			RemoveRappelComponent = true;
		}

		if (UseAimCameraSpring)
		{
			FollowCamera->AttachToComponent(Seat.OwningVehicle->GetMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Seat.SeatingSocketName);
			UpdateAimCamera();
		}

		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;

		HealthComp->SetCanBeWounded(false);

		IsInVehicle = true;
	}
	// otherwise no seat, in other words, not in a  vehicle
	else
	{
		if (UseAimCameraSpring)
		{
			FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;
		GetWorldTimerManager().SetTimer(THandler_CharacterMovement, this, &ABaseCharacter::UpdateCharacterMovement, .1f, true);
		
		IsInVehicle = false;
		SetIsExitingVehicle(false);

		HealthComp->SetCanBeWounded(HealthComp->GetCanBeWoundedDefault());

		// rappel component is no longer needed once exited vehicle.
		RemoveRappelComponent = true;
	}

	if (RemoveRappelComponent && RappellerComponent)
	{
		RappellerComponent->DestroyComponent();
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

void ABaseCharacter::SetIsReviving(bool Value)
{
	isReviving = Value;
	UHealthComponent::SetIsReviving(this, Value);
	// is in state of being revived
	if (isReviving)
	{
		Init(); // reset variables
		GetMesh()->SetSimulatePhysics(false);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionProfileName(DefaultMeshCollisionName);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		GetMesh()->SetRelativeLocationAndRotation(DefaultMeshLocation, DefaultMeshRotation);

		if (OverheadIcon) {
			OverheadIcon->HideIcon();
		}
	}
	else // has been revived
	{
		Revived();
	}
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
		if (IsInVehicle)
		{
			HealthComp->SetCanBeWounded(false);
		}

		VoiceAudioComponent->Stop();

		ClearTimeHandlers();

		ShowCharacterOutline(false, true);

		GetCharacterMovement()->StopMovementImmediately();

		// In case if died in vehicle.
		GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;

		// Custom collision profile to allow remove capsule collision but still run root motion
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Death"));



		EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(InHealthParameters.HitInfo.PhysMaterial.Get());

		// Play death sound if not recieved a headshot
		if (SurfaceType != SURFACE_HEAD)
		{
			if (VoiceClipsSet->DeathSound != nullptr)
			{
				VoiceAudioComponent->Activate();
				VoiceAudioComponent->Sound = VoiceClipsSet->DeathSound;
				VoiceAudioComponent->Play();
			}
		}

		PlayDeathAnim(InHealthParameters);

		// In case death anim asset is empty
		if (!DeathAnimationAsset) {
			DeathAnimationAsset = DeathAnimation->Defaults[rand() % DeathAnimation->Defaults.Num()];
		}


		if (InHealthParameters.AffectedHealthComponent->GetIsWounded())
		{
			if (OverheadIcon) {
				OverheadIcon->ShowIcon(EIconType::Wounded, false);
			}
		}
		else
		{
			GetWorldTimerManager().SetTimer(THandler_Destroyer, this, &ABaseCharacter::StartDestroy, DestroyDelayTime, false);
		}
		IsInVehicle = false;

		if (GetController()) {
			GetController()->UnPossess();
		}

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void ABaseCharacter::OnRappelChange(FRappellingParameters RappellingInfo)
{
	SetIsExitingVehicle(RappellingInfo.IsRappelling);

	if (RappellingInfo.IsComplete)
	{
		SetVehicleSeat(FVehicletSeating());
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

	//if (!HealthComp->IsAlive() && OtherActor != this)
	//{
	//	if (!HealthComp->GetIsWounded()) {
	//		PostDeath();
	//	}
	//}
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	auto Velocity = UKismetMathLibrary::Abs(GetVelocity().Size());

	auto NormalizedVelo = UKismetMathLibrary::NormalizeToRange(Velocity, 1000.f, 1500.f);

	auto ClampVelo = UKismetMathLibrary::ClampAngle(NormalizedVelo, .0f, 1.f);


	FHealthParameters HealthParameters;
	HealthParameters.DamageCauser = HealthParameters.DamagedActor = this;
	HealthParameters.Damage = ClampVelo * 100.f;
	HealthParameters.InstigatedBy = GetInstigatorController();
	HealthParameters.HitInfo = Hit;
	HealthComp->OnDamage(HealthParameters);

	FCharacterActionParameters CharacterActionParameters;
	CharacterActionParameters.Action = ECharacterAction::Landed;
	CharacterActionParameters.LandHeight = Velocity;
	OnCharacterActionUpdate.Broadcast(CharacterActionParameters);
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
	DeathAnimation = DeathAnimDatatable->FindRow<FDeathAnimation>(DeathAnimRowName, ContextString, true);
}

void ABaseCharacter::UpdateCameraView()
{
	if (IsFirstPersonView || !IsPlayerControlled()) {
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
		StopCover();
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

void ABaseCharacter::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);

	IsCurrentlyCrouched = false;
}

void ABaseCharacter::ToggleCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		if (isTakingCover)
		{
			// Can stand up while in cover?
			if (CanCoverStand())
			{
				UnCrouch();
			}
		}
		else
		{
			UnCrouch();
		}
	}
	else
	{
		Crouch();

		// to prevent stand to crouch animation from playing after leaving cover while still in cover state.
		if (isTakingCover)
		{
			IsCurrentlyCrouched = true;
		}
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
	float Roll = MoveToTarget.Roll;

	UKismetMathLibrary::BreakRotator(MoveToTarget, Roll, aimPitch, aimYaw);
}

void ABaseCharacter::UpdateCharacterMovement()
{
	UpdateSpeed();

	// check if character is in the air
	IsCharacterInAir = APawn::GetMovementComponent()->IsFalling();

	if (IsSprintDefault)
	{
		if (IsCharacterMoving() && !GetCharacterMovement()->IsCrouching() && !isTakingCover)
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
		AttachIconToHead(OverheadIcon);
	}
}

void ABaseCharacter::AttachIconToHead(AActor* Icon)
{
	Icon->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	FVector HeadLocation = GetMesh()->GetSocketLocation(GetHeadSocket());
	Icon->SetActorLocation(HeadLocation);
}

void ABaseCharacter::UpdateDirection()
{
	if (IsSprinting())
	{
		// We want the character to face towards camera's forward direction rather than actor's position,
		// this allows run and shoot to rotate towards camera direction
		auto TargetDirection =  FollowCamera->GetComponentRotation() - GetCapsuleComponent()->GetComponentRotation();
		auto Direction = TargetDirection.Yaw;
		CharacterDirection = Direction;
	}
	else
	{
		CharacterDirection = UKismetAnimationLibrary::CalculateDirection(GetVelocity(), GetActorRotation());
	}
}

bool ABaseCharacter::IsCharacterMoving()
{
	auto Velocity = GetCharacterMovement()->Velocity.Size();

	return Velocity > UKismetMathLibrary::Abs(1.f);
}

void ABaseCharacter::Jump()
{
	if (GetCharacterMovement()->IsCrouching() || isTakingCover) {
		return;
	}

	Super::Jump();

	FCharacterActionParameters CharacterActionParameters;
	CharacterActionParameters.Action = ECharacterAction::Jump;
	OnCharacterActionUpdate.Broadcast(CharacterActionParameters);
}

void ABaseCharacter::SetFirstPersonView()
{
	IsFirstPersonView = true;
	FollowCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocket);
}

void ABaseCharacter::ShowCharacterOutline(bool CanShow, bool IgnoreDeath)
{
	if (!HealthComp->IsAlive() && !IgnoreDeath) {
		if (!HealthComp->GetIsWounded()) {
			return;
		}
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


		if (LineTrace && OutHit.bBlockingHit)
		{
			// since character may already be crouched, set crouch flag to true.
			if (GetCharacterMovement()->IsCrouching())
			{
				StartCover(OutHit, true);
			}
			else
			{
				StartCover(OutHit, false);
			}
		}
		// if not found a trace from actor location, then trace from the knees or near bottom of character, but not tracing if currently crouched.
		else if (!GetCharacterMovement()->IsCrouching())
		{
			FVector StartBelow = Start;
			StartBelow.Z -= 50.f;

			FVector EndBelow = (GetActorForwardVector() * CoverDistance) + StartBelow;
			FHitResult OutHitBelow;
			LineTrace = GetWorld()->LineTraceSingleByChannel(OutHitBelow, StartBelow, EndBelow, COLLISION_COVER);

			if (LineTrace && OutHitBelow.bBlockingHit)
			{
				StartCover(OutHitBelow, true);
			}
		}
		else
		{
			StopCover();
		}
	}
}

void ABaseCharacter::StartCover(FHitResult OutHit, bool IsCrouchOnly)
{
	auto CoverGap = 5.f; // gap from cover point & character

	auto ForwardPoint = UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX((OutHit.ImpactPoint)));

	auto DistanceX = (OutHit.ImpactPoint.X + GetCapsuleComponent()->GetScaledCapsuleRadius()) + ForwardPoint.X + CoverGap;

	if (OutHit.ImpactNormal.X < 0) {
		DistanceX = (OutHit.ImpactPoint.X - GetCapsuleComponent()->GetScaledCapsuleRadius()) - ForwardPoint.X - CoverGap;
	}

	auto DistanceY = (OutHit.ImpactPoint.Y + GetCapsuleComponent()->GetScaledCapsuleRadius()) + ForwardPoint.Y + CoverGap;

	if (OutHit.ImpactNormal.Y < 0) {
		DistanceY = (OutHit.ImpactPoint.Y - GetCapsuleComponent()->GetScaledCapsuleRadius()) - ForwardPoint.Y - CoverGap;
	}

	FVector CoverFirstPos = FVector(
		DistanceX,
		DistanceY,
		GetActorLocation().Z
	);

	// Only rotate in the horizontal axis to cover point.
	LastCoverPosition = CoverFirstPos;
	LastCoverRotation = UKismetMathLibrary::MakeRotFromZ(OutHit.ImpactNormal);
	LastCoverRotation.Roll = 0.f;
	LastCoverRotation.Pitch = 0.f;
	LastCoverRotation.Yaw += CoverStartYawOffset;

	IsCurrentlyCrouched = IsCrouchOnly;

	SetActorLocationAndRotation(CoverFirstPos, LastCoverRotation);

	if (IsCrouchOnly)
	{
		Crouch();
	}
	else
	{
		//FLatentActionInfo LatentInfo;
		//LatentInfo.CallbackTarget = this;

		//UKismetSystemLibrary::MoveComponentTo(
		//	GetCapsuleComponent(),
		//	CoverFirstPos,
		//	LastCoverRotation,
		//	false,
		//	false,
		//	.2f,
		//	false,
		//	EMoveComponentAction::Type::Move,
		//	LatentInfo
		//);
	}

	EndAim();

	GetCharacterMovement()->SetPlaneConstraintEnabled(true);
	GetCharacterMovement()->SetPlaneConstraintNormal(OutHit.Normal);
	GetCharacterMovement()->bOrientRotationToMovement = false;

	bUseControllerRotationYaw = true;
	isTakingCover = true;

	bool LineTraceLeft = false;
	bool LineTraceRight = false;
	GetCorners(OutHit.Normal, LineTraceLeft, LineTraceRight);

	if (!LineTraceLeft)
	{
		RotateToLeftCorner();
	}
	else if (!LineTraceRight)
	{
		RotateToRightCorner();
	}
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

bool ABaseCharacter::CanCoverStand()
{
	FVector Start = GetActorLocation();
	Start.Z += 50.f;
	FVector End = ((GetActorForwardVector() * -1.f) * CoverDistance) + Start;

	FHitResult OutHit;
	bool LineTrace = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, COLLISION_COVER);

	// if has hit a cover collision layer then character can stand up.
	return LineTrace && OutHit.bBlockingHit;
}

void ABaseCharacter::CoverMovement(float Value)
{
	bool IsUpdated = false;

	FVector WallDirection = GetCharacterMovement()->GetPlaneConstraintNormal() * -1.0f; // get direction towards the cover wall

	bool LineTraceRight = false;
	bool LineTraceLeft = false;

	GetCorners(GetCharacterMovement()->GetPlaneConstraintNormal(), LineTraceLeft, LineTraceRight);

	FVector Dir = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotator(0.0f, 0.0f, GetControlRotation().Yaw));
	LastCoverPosition = GetActorLocation();

	// if no line cover found for both sides then assuming character is not in cover.
	if (!LineTraceLeft && !LineTraceRight)
	{
		StopCover();
	}

	// if can move left or right in cover.
	if (LineTraceLeft || LineTraceRight)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false; // based on corner animation being used, set to false
	}

	// if has left & right points to move to.
	if (LineTraceLeft && LineTraceRight)
	{
		isAtCoverCorner = false;

		FVector Start = GetActorLocation();
		FVector End = Start + (WallDirection * CoverDistance);
		FHitResult OutHit;

		bool LineTrace = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, COLLISION_COVER);

		if (LineTrace && OutHit.bBlockingHit)
		{
			FRotator NewRotation = UKismetMathLibrary::MakeRotFromX(OutHit.Normal * -1.f);
			NewRotation.Roll = 0.f;
			NewRotation.Pitch = 0.f;
			NewRotation.Yaw += CoverMovementYawOffset;

			SetActorRotation(NewRotation);
			GetCharacterMovement()->SetPlaneConstraintNormal(OutHit.Normal);
			AddMovementInput(Dir, Value);

			IsUpdated = true;
		}

	}
	else if (LineTraceLeft) // if reached right corner
	{
		if (Value < 0.0f)
		{
			AddMovementInput(Dir, Value);
		}

		RotateToRightCorner();
		IsUpdated = true;
	}
	else if (LineTraceRight) // if reached left corner
	{
		if (Value > 0.0f)
		{
			AddMovementInput(Dir, Value);
		}

		RotateToLeftCorner();
		IsUpdated = true;
	}

	if (IsUpdated)
	{
		FCoverUpdateInfo CoverUpdateInfo;
		CoverUpdateInfo.RightInputValue = Value;
		OnCoverUpdate.Broadcast(CoverUpdateInfo);
	}

}

void ABaseCharacter::GetCorners(FVector WallNormal, bool& LineTraceLeft, bool& LineTraceRight)
{
	FVector WallDirection = WallNormal * -1.0f; // get direction towards the cover wall
	FVector StartLocation = GetActorLocation();

	FVector RightVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallDirection)) * GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector StartRight = StartLocation + RightVector;
	FVector EndRight = WallDirection * CoverDistance + StartRight;
	FHitResult OutHitRight;
	LineTraceRight = GetWorld()->LineTraceSingleByChannel(OutHitRight, StartRight, EndRight, COLLISION_COVER);

	FVector LeftVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallNormal)) * GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector StartLeft = StartLocation + LeftVector;
	FVector EndLeft = WallDirection * CoverDistance + StartLeft;
	FHitResult OutHitLeft;
	LineTraceLeft = GetWorld()->LineTraceSingleByChannel(OutHitLeft, StartLeft, EndLeft, COLLISION_COVER);

	// try line tracing for crouching corners if not found line traces for stand state.
	StartLocation.Z -= 50.f;

	if (!LineTraceRight)
	{
		StartRight = StartLocation + RightVector;
		EndRight = WallDirection * CoverDistance + StartRight;
		FHitResult OutHitBelow;
		LineTraceRight = GetWorld()->LineTraceSingleByChannel(OutHitBelow, StartRight, EndRight, COLLISION_COVER);
	}

	if (!LineTraceLeft)
	{
		StartLeft = StartLocation + LeftVector;
		EndLeft = WallDirection * CoverDistance + StartLeft;
		FHitResult OutHitBelow;
		LineTraceLeft = GetWorld()->LineTraceSingleByChannel(OutHitBelow, StartLeft, EndLeft, COLLISION_COVER);
	}
}

void ABaseCharacter::RotateToLeftCorner()
{
	FVector WallDirection = GetCharacterMovement()->GetPlaneConstraintNormal() * -1.0f; // get direction towards the cover wall

	isAtCoverCorner = true;
	isFacingCoverRHS = false;

	LastCoverRotation = UKismetMathLibrary::MakeRotFromZ(WallDirection);
	LastCoverRotation.Roll = 0.f;
	LastCoverRotation.Pitch = 0.f;
	LastCoverRotation.Yaw += CoverCornerLeftYawOffset;

	SetActorRotation(LastCoverRotation);
	FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);
}

void ABaseCharacter::RotateToRightCorner()
{
	isAtCoverCorner = true;
	isFacingCoverRHS = true;

	LastCoverRotation = UKismetMathLibrary::MakeRotFromZ(GetCharacterMovement()->GetPlaneConstraintNormal());
	LastCoverRotation.Roll = 0.f;
	LastCoverRotation.Pitch = 0.f;
	LastCoverRotation.Yaw += CoverCornerRightYawOffset;

	SetActorRotation(LastCoverRotation);
	FollowCamera->SetRelativeRotation(FRotator::ZeroRotator);
}

bool ABaseCharacter::CanAim()
{
	if (isTakingCover)
	{
		if (isAtCoverCorner)
		{
			return true;
		}
		else
		{
			if (CanCoverPeakUp()) {
				return true;
			}
			else {
				return false;
			}
		}
	}
	else
	{
		return true;
	}
}

bool ABaseCharacter::CanCoverPeakUp()
{
	if (!isTakingCover) {
		return false;
	}

	FVector WallDirection = GetCharacterMovement()->GetPlaneConstraintNormal() * -1.0f; // get direction towards the cover wall

	FVector RightVector = UKismetMathLibrary::GetRightVector(UKismetMathLibrary::MakeRotFromX(WallDirection));
	FVector Start = RightVector + GetMesh()->GetSocketLocation(HeadSocket) + CoverPeakUpOffset;
	FVector End = WallDirection * CoverDistance + Start;

	FHitResult OutHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	bool LineTrace = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	return !LineTrace;
}


void ABaseCharacter::BeginAim()
{
	if (!CanAim()) {
		return;
	}

	isAiming = true;

	if (isAiming && UseAimCameraSpring && !IsInVehicle)
	{
		// switch between camera angles based on which side of the cover the character is using. (left or right cover)
		if (isTakingCover) {
			if (isAtCoverCorner) {
				if (isFacingCoverRHS) {
					FollowCamera->AttachToComponent(AimCameraRightSpring, FAttachmentTransformRules::KeepWorldTransform);
				}
				else {
					FollowCamera->AttachToComponent(AimCameraLeftSpring, FAttachmentTransformRules::KeepWorldTransform);
				}
			}
		}
		else
		{
			FollowCamera->AttachToComponent(AimCameraRightSpring, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}


		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector::ZeroVector, FRotator::ZeroRotator, true, true, 0.3f, false, EMoveComponentAction::Type::Move, LatentInfo);

		FCharacterActionParameters CharacterActionParameters;
		CharacterActionParameters.Action = ECharacterAction::BeginAim;
		OnCharacterActionUpdate.Broadcast(CharacterActionParameters);
	}

}

void ABaseCharacter::EndAim()
{
	if (!isAiming) {
		return;
	}
	isAiming = false;

	if (UseAimCameraSpring && !IsInVehicle)
	{
		FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepWorldTransform);

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;

		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector::ZeroVector, FRotator::ZeroRotator, true, true, 0.3f, false, EMoveComponentAction::Type::Move, LatentInfo);
	
		FCharacterActionParameters CharacterActionParameters;
		CharacterActionParameters.Action = ECharacterAction::BeginAim;
		OnCharacterActionUpdate.Broadcast(CharacterActionParameters);
	}
}

void ABaseCharacter::UpdateAimCamera()
{
	FollowCamera->SetWorldLocation(GetMesh()->GetSocketLocation(ShoulderRightSocket));
}

void ABaseCharacter::TraceFootstep()
{
	if (CharacterSpeed <= 0.f) {
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;


	FVector LeftFootPos = GetMesh()->GetSocketLocation(LeftFootSocket);
	FVector LeftFootEnsPos = FVector(LeftFootPos.X, LeftFootPos.Y, LeftFootPos.Z - 5);

	FHitResult OutHitLeft;
	auto LeftFootTrace = GetWorld()->LineTraceSingleByObjectType(OutHitLeft, LeftFootPos, LeftFootEnsPos, ObjectParams, QueryParams);

	if (LeftFootTrace)
	{
		PlayFootstepSound(OutHitLeft);
	}

	FVector RightFootPos = GetMesh()->GetSocketLocation(RightFootSocket);
	FVector RightFootEnsPos = FVector(RightFootPos.X, RightFootPos.Y, RightFootPos.Z - 5);

	FHitResult OutHitRight;
	auto RightFootTrace = GetWorld()->LineTraceSingleByObjectType(OutHitRight, RightFootPos, RightFootEnsPos, ObjectParams, QueryParams);

	if (RightFootTrace)
	{
		PlayFootstepSound(OutHitRight);
	}
}

void ABaseCharacter::PlayFootstepSound(FHitResult HitInfo)
{
	USoundBase* Sound = GetFootstepSound(HitInfo);

	if (!Sound) {
		return;
	}

	FName DirectionParam = "is_move_right";
	int32 DirectionVal = CharacterDirection >= 0 ? 1 : 0;

	FName StanceParam = "stance";
	int32 StanceVal = 0;

	if (isSprinting)
	{
		StanceVal = 1;
	}
	else if (GetCharacterMovement()->IsCrouching())
	{
		StanceVal = 2;
	}

	auto FootstepAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, HitInfo.ImpactPoint);

	if (FootstepAudioComponent)
	{
		FootstepAudioComponent->SetIntParameter(StanceParam, StanceVal);
		FootstepAudioComponent->SetIntParameter(DirectionParam, DirectionVal);
	}

}

USoundBase* ABaseCharacter::GetFootstepSound(FHitResult HitInfo)
{
	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitInfo.PhysMaterial.Get());

	FSurfaceImpactSet SurfaceImpactSet = FootSurfaceImpact->Default;

	if (HitInfo.PhysMaterial == nullptr) {
		return SurfaceImpactSet.Sound;
	}

	switch (SurfaceType)
	{
	case SURFACE_CONCRETE:
		SurfaceImpactSet = FootSurfaceImpact->Concrete;
		break;
	case SURFACE_WATER:
		SurfaceImpactSet = FootSurfaceImpact->Water;
		break;
	case SURFACE_GRASS:
		SurfaceImpactSet = FootSurfaceImpact->Grass;
		break;
	case SURFACE_WOOD:
		SurfaceImpactSet = FootSurfaceImpact->Wood;
		break;
	case SURFACE_ROCK:
		SurfaceImpactSet = FootSurfaceImpact->Rock;
		break;
	case SURFACE_SAND:
		SurfaceImpactSet = FootSurfaceImpact->Sand;
		break;
	default:
		SurfaceImpactSet = FootSurfaceImpact->Default;
		break;
	}

	return SurfaceImpactSet.Sound;
}

void ABaseCharacter::PlayVoiceSound(USoundBase* Sound)
{
	if (!HealthComp->IsAlive() || Sound == nullptr) {
		return;
	}

	VoiceAudioComponent->Activate();
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
			if (HasHitLeft && !DeathAnimation->SprintExplosionsRight.IsEmpty())
			{
				DeathAnimationAsset = DeathAnimation->SprintExplosionsRight[rand() % DeathAnimation->SprintExplosionsRight.Num()];
			}
			else if (HasHitRight && !DeathAnimation->SprintExplosionsLeft.IsEmpty())
			{
				DeathAnimationAsset = DeathAnimation->SprintExplosionsLeft[rand() % DeathAnimation->SprintExplosionsLeft.Num()];
			}
			else if (!DeathAnimation->SprintExplosionsFront.IsEmpty())
			{
				DeathAnimationAsset = DeathAnimation->SprintExplosionsFront[rand() % DeathAnimation->SprintExplosionsFront.Num()];
			}
		}
		else
		{
			if (HasHitFront && !DeathAnimation->StandExplosionsBack.IsEmpty())
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsBack[rand() % DeathAnimation->StandExplosionsBack.Num()];
			}
			else if (HasHitLeft && !DeathAnimation->StandExplosionsRight.IsEmpty())
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsRight[rand() % DeathAnimation->StandExplosionsRight.Num()];
			}
			else if (HasHitRight && !DeathAnimation->StandExplosionsLeft.IsEmpty())
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsLeft[rand() % DeathAnimation->StandExplosionsLeft.Num()];
			}
			else if (!DeathAnimation->StandExplosionsFront.IsEmpty())
			{
				DeathAnimationAsset = DeathAnimation->StandExplosionsFront[rand() % DeathAnimation->StandExplosionsFront.Num()];
			}
		}
		return;
	}

	if (InHealthParameters.WeaponCauser && InHealthParameters.WeaponCauser->GetWeaponType() == WeaponType::Shotgun)
	{
		if (SurfaceType == SURFACE_LEGS && !DeathAnimation->ShotgunHitsLegs.IsEmpty())
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsLegs[rand() % DeathAnimation->ShotgunHitsLegs.Num()];
			return;
		}
		else if (HasHitFront && !DeathAnimation->ShotgunHitsFront.IsEmpty())
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsFront[rand() % DeathAnimation->ShotgunHitsFront.Num()];
			return;
		}
		else if (HasHitLeft && !DeathAnimation->ShotgunHitsLeft.IsEmpty())
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsLeft[rand() % DeathAnimation->ShotgunHitsLeft.Num()];
			return;
		}
		else if (HasHitRight && !DeathAnimation->ShotgunHitsRight.IsEmpty())
		{
			DeathAnimationAsset = DeathAnimation->ShotgunHitsRight[rand() % DeathAnimation->ShotgunHitsRight.Num()];
			return;
		}
	}

	if (isSprinting && !DeathAnimation->Sprints.IsEmpty())
	{
		int RandomIndex = rand() % DeathAnimation->Sprints.Num();
		DeathAnimationAsset = DeathAnimation->Sprints[RandomIndex];
		return;
	}

	if (GetCharacterMovement()->IsCrouching() && !DeathAnimation->Crouches.IsEmpty())
	{
		int RandomIndex = rand() % DeathAnimation->Crouches.Num();
		DeathAnimationAsset = DeathAnimation->Crouches[RandomIndex];
		return;
	}


	switch (SurfaceType)
	{
	case SURFACE_HEAD:
		if (!DeathAnimation->Headshots.IsEmpty())
		{
			DeathAnimationAsset = DeathAnimation->Headshots[rand() % DeathAnimation->Headshots.Num()];
		}
		return;
	case SURFACE_FLESHVULNERABLE:
		if (!DeathAnimation->Vulernables.IsEmpty())
		{
			DeathAnimationAsset = DeathAnimation->Vulernables[rand() % DeathAnimation->Vulernables.Num()];
		}
		return;
	case SURFACE_GROIN:
		if (!DeathAnimation->Groins.IsEmpty())
		{
			DeathAnimationAsset = DeathAnimation->Groins[rand() % DeathAnimation->Groins.Num()];
		}
		return;
	default:
		DeathAnimationAsset = DeathAnimation->Defaults[rand() % DeathAnimation->Defaults.Num()];
		return;
	}
}

void ABaseCharacter::HandleVoiceAudioFinished()
{
	VoiceAudioComponent->Deactivate();
}

void ABaseCharacter::PostDeath()
{
	//GetMesh()->SetAnimInstanceClass(NULL);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Death"));
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DestroyUnusedComponents()
{
	if (AimCameraRightSpring) AimCameraRightSpring->DestroyComponent();
	if (AimCameraLeftSpring) AimCameraLeftSpring->DestroyComponent();
	if (CameraBoom) CameraBoom->DestroyComponent();
	if (RappellerComponent && !IsInVehicle) RappellerComponent->DestroyComponent();
}

void ABaseCharacter::StartDestroy()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	DetroyChildActor(AttachedActors);

	SetActorHiddenInGame(true);
	SetHidden(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	//Destroy();
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
