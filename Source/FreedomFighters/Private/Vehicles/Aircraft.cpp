#include "Vehicles/Aircraft.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"
#include "Props/VehicleSplinePath.h"
#include "Accessories/Rope.h"
#include "Props/TargetSystemMarker.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/WeaponBullet.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/ObjectPoolComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SplineComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "AIController.h"
#include "NavigationSystem.h"

AAircraft::AAircraft()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root"));
	RootComponent = CapsuleComponent;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("Vehicle"));
	Mesh->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);

	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);


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

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	ThermalVisionPPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ThermalVisionPPComp"));
	ThermalVisionPPComp->SetupAttachment(FollowCamera);

	// follow will contain any interior sounds so these sounds cannot be heard outside of the aircraft
	PilotAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PilotAudio"));
	PilotAudio->SetupAttachment(FollowCamera);

	ThermalToggleAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ThermalToggleAudio"));
	ThermalToggleAudio->SetupAttachment(FollowCamera);


	WingRotationSpeed = 100000.0;
	PathFollowDuration = 10.0f;
	TotalLaps = 0;
	CurrentWeaponIndex = 0;
	CameraSwitchDelay = .5f;
	RandomNavPointRadius = 1000.f;

	UseFollowCamNavigation = false;
	HighlightCharacters = false;
	FollowTargetLocation = false;

	KillConfirmedParamName = "KillConfirmed";
	SingleKillIndex = 0;
	DoubleKillIndex = 1;
	MultiKillIndex = 2;

	SpawnLocationMin = FVector(-10.f, -10.f, 10.f);
	SpawnLocationMax = FVector(10.f, 10.f, 10.f);
}

void AAircraft::AddUIWidget()
{
	UWorld* World = GetWorld();

	if (!World) return;

	UWidgetLayoutLibrary::RemoveAllWidgets(World);

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(World, HUDWidgetClass);

		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

void AAircraft::RemoveUIWidget()
{
	UWorld* World = GetWorld();

	if (!World) return;

	if (HUDWidget)
	{
		UWidgetLayoutLibrary::RemoveAllWidgets(World);
		HUDWidget = nullptr;
	}
}


void AAircraft::BeginPlay()
{
	Super::BeginPlay();

	EngineAudio->Play();

	ThermalVisionPPComp->bEnabled = false;
	CreateThermalMatInstances();

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AAircraft::OnOverlapBegin);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AAircraft::OnOverlapBegin);

	FindPath();

	SpawnWeapon();

	SpawnPassenger();

	// Call after spawning passengers, otherwise character movement component will force characters to be on the ground
	SpawnRandomLocation();
}

void AAircraft::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurveTimeline.TickTimeline(DeltaTime);

	CurrentWingSpeed = WingRotationSpeed * DeltaTime;

	SetTargetSystem();

	WaitForRapelling();

	UpdateOccupiedSeats();
}

void AAircraft::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AircraftPath != nullptr)
	{
		FVector CollisionLocation = GetActorLocation();
		auto CollidedPath = Cast<AVehicleSplinePath>(OtherActor);

		// if collided with another aircraft path then ignore then do not proceed
		if (CollidedPath != AircraftPath) {
			return;
		}

		// find current spline point
		FVehicleSplinePoint CurrentSplinePoint = FVehicleSplinePoint();

		// if current spline point not found 
		if (CurrentSplinePoint.PointIndex == -1) {
			return;
		}

		// update the aircraft movement type
		CurrentAircraftMovement = CurrentSplinePoint.MovementType;

		// adjust path duration to change speed if specified
		if (CurrentSplinePoint.AffectSpeedType == EVehicleSpeedType::Specified)
		{

			CurveTimeline.SetPlayRate(1.0f / CurrentSplinePoint.PathDuration);
		}
		else
		{
			CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
		}

		if (CurrentSplinePoint.IsPathFreeToUse)
		{
			AircraftPath->SetOccupantVehicle(nullptr);
		}

	}
}

void AAircraft::FindPath()
{
	if (FollowTargetLocation)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("MoveToLocation"));
		CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		CurveTimeline.SetLooping(false);
		CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
		CurveTimeline.PlayFromStart();
	}
	else
	{
		// if already assigned a path then return
		if (AircraftPath != nullptr)
		{
			return;
		}

		AVehicleSplinePath* ClosestPath = nullptr;
		float ClosestDistance = 0.0f;
		TArray<AActor*> TargetActor;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), AircraftPathTagName, TargetActor);

		for (AActor* Actor : TargetActor)
		{
			auto Path = Cast<AVehicleSplinePath>(Actor);

			// check if path is not occupied
			if (Path && !Path->GetOccupiedVehicle())
			{
				float Distance = UKismetMathLibrary::Vector_Distance(TargetDestination, Path->GetActorLocation());

				if (ClosestPath == nullptr)
				{
					ClosestPath = Path;
					ClosestDistance = Distance;
				}
				else
				{
					if (Distance < ClosestDistance)
					{
						ClosestPath = Path;
						ClosestDistance = Distance;
					}
				}
			}

		}

		if (ClosestPath)
		{
			AircraftPath = ClosestPath;
			AircraftPath->SetOccupantVehicle(this);

			// setup time line for following the path
			if (CurveFloat)
			{
				FOnTimelineFloat TimelineProgress;
				TimelineProgress.BindUFunction(this, FName("FollowSplinePath"));
				CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
				CurveTimeline.SetLooping(false);
				CurveTimeline.SetPlayRate(1.0f / PathFollowDuration);
				CurveTimeline.PlayFromStart();
			}
		}
	}

}

void AAircraft::FollowSplinePath(float Value)
{
	USplineComponent* SplinePathComp = AircraftPath->GetSplinePathComp();
	float Alpha = UKismetMathLibrary::Lerp(0.0f, SplinePathComp->GetSplineLength(), Value);

	FVector TargetLocation = SplinePathComp->GetLocationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);
	FRotator TargetRotation = SplinePathComp->GetRotationAtDistanceAlongSpline(Alpha, ESplineCoordinateSpace::World);

	SetActorLocationAndRotation(TargetLocation, TargetRotation);

	// if reached the end
	if (Alpha >= SplinePathComp->GetSplineLength())
	{
		if (CurrentLap < TotalLaps) // restart the lap if laps remaining
		{
			CurrentLap++;
			CurveTimeline.PlayFromStart();
		}
		else
		{
			Destroy();
		}
	}
}

void AAircraft::MoveToLocation(float Value)
{
	float TargetX = UKismetMathLibrary::Lerp(GetActorLocation().X, TargetDestination.X, Value);
	float TargetY = UKismetMathLibrary::Lerp(GetActorLocation().Y, TargetDestination.Y, Value);

	SetActorLocation(FVector(TargetX, TargetY, GetActorLocation().Z));

	if (Value >= 1.f)
	{
		CurrentAircraftMovement = TargetLocReachedAircraftMovementType;
	}
}

void AAircraft::OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters)
{
	if (ProjectileImpactParameters.KillCount <= 0) {
		return;
	}

	// Prioritise the kill confirmed sounds over random voice sounds
	if (RandomPilotSound && PilotAudio->Sound == RandomPilotSound)
	{
		PilotAudio->Stop();
	}

	// allow the initial voice sounds to play before playing kill confirmed sounds
	if (!PilotAudio->IsPlaying())
	{
		PilotAudio->Sound = KillConfirmedSound;

		if (ProjectileImpactParameters.IsSingleKill)
		{
			PilotAudio->SetIntParameter(KillConfirmedParamName, SingleKillIndex);
		}
		else if (ProjectileImpactParameters.IsDoubleKill)
		{
			PilotAudio->SetIntParameter(KillConfirmedParamName, DoubleKillIndex);
		}
		else if (ProjectileImpactParameters.IsMultiKill)
		{
			PilotAudio->SetIntParameter(KillConfirmedParamName, MultiKillIndex);
		}
		PilotAudio->Play();
	}
}

void AAircraft::SpawnPassenger()
{
	if (AircraftSeats.Num() <= 0) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < AircraftSeats.Num(); i++)
	{
		FAircraftSeating HeliSeat = AircraftSeats[i];

		if (HeliSeat.Character)
		{
			HeliSeat.CharacterObj = GetWorld()->SpawnActor<ABaseCharacter>(HeliSeat.Character, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (HeliSeat.CharacterObj)
			{
				HeliSeat.OwningAircraft = this;

				ABaseCharacter* Character = HeliSeat.CharacterObj;
				Character->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeliSeat.SeatingSocketName);
				//Character->SetAircraftSeat(HeliSeat);

				// Set to use weapon, usually a mounted gun would be used
				if (HeliSeat.AssociatedWeapon > -1)
				{
					if (Cast<AMountedGun>(WeaponObjs[HeliSeat.AssociatedWeapon])) // if it's a mounted gun
					{
						ACombatCharacter* CombatCharacter = Cast<ACombatCharacter>(Character);
						CombatCharacter->SetMountedGun(WeaponObjs[HeliSeat.AssociatedWeapon]);
						CombatCharacter->UseMountedGun();
					}
				}

				OccupiedSeats.Add(HeliSeat);
			}
		}
	}
}

void AAircraft::SpawnRandomLocation()
{
	FindNearestNav();

	// Set for random spawn location from target location
	if (FollowTargetLocation)
	{
		float X = UKismetMathLibrary::RandomFloatInRange(SpawnLocationMin.X * TargetDestination.X, SpawnLocationMax.X * TargetDestination.X);
		float Y = UKismetMathLibrary::RandomFloatInRange(SpawnLocationMin.Y * TargetDestination.Y, SpawnLocationMax.Y * TargetDestination.Y);
		float Z = UKismetMathLibrary::RandomFloatInRange(SpawnLocationMin.Z * TargetDestination.Z, SpawnLocationMax.Z * TargetDestination.Z);

		SetActorLocation(FVector(X, Y, Z));

		FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetDestination);

		// Look towards target location
		SetActorRotation(TargetRot);
	}
}

void AAircraft::FindNearestNav()
{
	FVector TargetDest = TargetDestination;
	FNavLocation NavLocation;
	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);
	bool navResult = NavigationArea->GetRandomReachablePointInRadius(TargetDest, RandomNavPointRadius, NavLocation);

	if (navResult)
	{
		TargetDestination = NavLocation.Location;
	}
}

void AAircraft::SpawnWeapon()
{
	if (AircraftWeapons.Num() <= 0) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (FAircraftWeapon AircraftWeapon : AircraftWeapons)
	{
		AMountedGun* MG = GetWorld()->SpawnActor<AMountedGun>(AircraftWeapon.Weapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (MG)
		{
			MG->getMeshComp()->SetCollisionProfileName(TEXT("NoCollision")); // To allow line trace to go ignore this weapon, for AI sights for example
			MG->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AircraftWeapon.WeaponSocketName);
			MG->GetClipAudioComponent()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			MG->GetShotAudioComponent()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			// we do not want to adjust character behind MG as character will be spawning behind it
			// & remove line trace mechanic for it so only spawned characters can use it
			// & gunner cannot exit from gun

			MG->SetAdjustBehindMG(false);
			MG->SetCanTraceInteraction(false);
			MG->SetCanExit(false);

			MG->SetPitchMin(AircraftWeapon.PitchMin);
			MG->SetPitchMax(AircraftWeapon.PitchMax);
			MG->SetYawMin(AircraftWeapon.YawMin);
			MG->SetYawMax(AircraftWeapon.YawMax);

			MG->OnKillConfirmed.AddDynamic(this, &AAircraft::OnWeaponKillConfirm);

			WeaponObjs.Add(MG);
		}
	}

	UpdateWeaponView();
}


void AAircraft::ChangeWeapon()
{
	if (AircraftWeapons.Num() <= 0) {
		return;
	}

	// increment the index if current index is less than the array of weapons
	// otherwise go back to the first index
	if (CurrentWeaponIndex < AircraftWeapons.Num() - 1)
	{
		CurrentWeaponIndex++;
	}
	else
	{
		CurrentWeaponIndex = 0;
	}

	if (CurrentWeaponObj) {
		CurrentWeaponObj->StopFire(); // stop firing current weapon before switching to another
	}

	UpdateWeaponView();
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetViewTargetWithBlend(CurrentWeaponObj);

}

void AAircraft::UpdateWeaponView()
{
	// return previous characters back to AI posession
	if (OccupiedSeats.Num() > 0 && CurrentWeaponIndex - 1 >= 0)
	{
		FAircraftSeating HeliSeat = OccupiedSeats[CurrentWeaponIndex - 1];
		HeliSeat.CharacterObj->AutoPossessPlayer = EAutoReceiveInput::Disabled;
		if (HeliSeat.CharacterObj->GetDefaultAIController())
		{
			HeliSeat.CharacterObj->GetDefaultAIController()->Possess(HeliSeat.CharacterObj);
		}
		//HeliSeat.CharacterObj->SetAircraftSeat(HeliSeat); // so AI does not fall to the ground when repossessed
	}

	CurrentAircraftWeapon = AircraftWeapons[CurrentWeaponIndex]; // set the current weapon to first weapon by default
	CurrentWeaponObj = WeaponObjs[CurrentWeaponIndex];

	CurrentWeaponObj->StopFire();

	// Player possessing is required if the character is posseseed by the AI controller
	if (OccupiedSeats.Num() > 0)
	{
		FAircraftSeating HeliSeat = OccupiedSeats[CurrentWeaponIndex];
		HeliSeat.CharacterObj->GetDefaultAIController()->UnPossess();
		HeliSeat.CharacterObj->AutoPossessPlayer = EAutoReceiveInput::Player0;
		HeliSeat.CharacterObj->EndAim();
	}

	if (UseFollowCamNavigation)
	{
		FollowCamera->AttachToComponent(CurrentWeaponObj->getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeaponObj->GetCameraPositionSocket());
		CurrentWeaponObj->GetFollowCamera()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FollowCamera->SetRelativeRotation(RotationInput);
		FollowCamera->SetFieldOfView(CurrentWeaponObj->GetFollowCamera()->FieldOfView);
	}
	else
	{
		FollowCamera->AttachToComponent(CurrentWeaponObj->GetFollowCamera(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	OnUpdateWeaponUI.Broadcast(this);
}

/// <summary>
/// Weapon control
/// </summary>
/// <param name="Val"></param>
void AAircraft::AddControllerPitchInput(float Val)
{
	if (UseFollowCamNavigation)
	{
		RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, CurrentAircraftWeapon.PitchMin, CurrentAircraftWeapon.PitchMax);
		RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

		FollowCamera->SetRelativeRotation(RotationInput);
	}
	else
	{
		CurrentWeaponObj->AddControllerPitchInput(Val);
		RotationInput = CurrentWeaponObj->GetRotationInput();
	}
}
void AAircraft::AddControllerYawInput(float Val)
{
	if (UseFollowCamNavigation)
	{
		RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, CurrentAircraftWeapon.YawMin, CurrentAircraftWeapon.YawMax);
		RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

		FollowCamera->SetRelativeRotation(RotationInput);
	}
	else
	{
		CurrentWeaponObj->AddControllerYawInput(Val);
		RotationInput = CurrentWeaponObj->GetRotationInput();
	}
}

/// <summary>
/// Third person view of the aircraft control
/// </summary>
/// <param name="Val"></param>
/// <param name="IsCameraRoam"></param>
void AAircraft::AddControllerPitchInput(float Val, bool IsCameraRoam)
{
	RotationInput.Pitch += Val;

	CameraBoom->SetWorldRotation(RotationInput);
}
void AAircraft::AddControllerYawInput(float Val, bool IsCameraRoam)
{
	RotationInput.Yaw += Val;

	CameraBoom->SetWorldRotation(RotationInput);
}

/// <summary>
/// Passenger Pitch Control
/// </summary>
/// <param name="Val"></param>
/// <param name="AircraftSeating"></param>
void AAircraft::AddControllerPitchInput(float Val, FAircraftSeating AircraftSeating)
{
	RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, AircraftSeating.PitchMin, AircraftSeating.PitchMax);
	RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	AircraftSeating.CharacterObj->FollowCamera->SetRelativeRotation(RotationInput);
}

void AAircraft::AddControllerYawInput(float Val, FAircraftSeating AircraftSeating)
{
	RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, AircraftSeating.YawMin, AircraftSeating.YawMax);
	RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	AircraftSeating.CharacterObj->FollowCamera->SetRelativeRotation(RotationInput);
}

void AAircraft::BeginAim()
{
	CurrentWeaponObj->SetIsAiming(true);
}

void AAircraft::EndAim()
{
	CurrentWeaponObj->SetIsAiming(false);
}

void AAircraft::ShowOutlines(bool CanShow)
{
	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Characters);

	for (AActor* Actor : Characters)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(Actor);

		if (Character != nullptr)
		{
			Character->ShowCharacterOutline(CanShow);
		}
	}

	TArray<AActor*> SkeletalMeahComps;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), USkeletalMeshComponent::StaticClass(), SkeletalMeahComps);

	for (AActor* Actor : SkeletalMeahComps)
	{
		USkeletalMeshComponent* currentSkel = Cast<USkeletalMeshComponent>(Actor);

		if (currentSkel != nullptr)
		{
			currentSkel->SetRenderCustomDepth(CanShow);
		}
	}

	TArray<AActor*> StaticComponents;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), UStaticMeshComponent::StaticClass(), StaticComponents);

	for (AActor* Actor : StaticComponents)
	{
		UStaticMeshComponent* currentstatic = Cast<UStaticMeshComponent>(Actor);

		if (currentstatic != nullptr)
		{
			currentstatic->SetRenderCustomDepth(CanShow);
		}
	}

}

void AAircraft::SetTargetSystem()
{
	if (FriendlyMarkerClass == nullptr && EnemyMarkerClass == nullptr) {
		return;
	}

	AActor* MyOwner = GetOwner();

	if (!MyOwner) {
		return;
	}

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);

	// adding characters to targetting
	for (AActor* Actor : Actors)
	{
		bool IsFactionCompActive = UTeamFactionComponent::IsComponentActive(Actor);

		if (IsFactionCompActive)
		{
			bool isFriendly = UTeamFactionComponent::IsFriendly(MyOwner, Actor);

			if (isFriendly)
			{
				if (!DoesFriendlyNodeExists(Actor))
				{
					FTargetSystemNode* TargetNode = new FTargetSystemNode;
					TargetNode->Actor = Actor;
					FriendlyMarkerNodes.Add(TargetNode);
				}
			}
			else
			{
				if (!DoesEnemyNodeExists(Actor))
				{
					FTargetSystemNode* TargetNode = new FTargetSystemNode;
					TargetNode->Actor = Actor;
					EnemySystemNodes.Add(TargetNode);
				}
			}
		}
	}

	if (HighlightCharacters)
	{
		ShowOutlines(true);
	}

	UpdateMarker(FriendlyMarkerNodes, FriendlyMarkerClass);
	UpdateMarker(EnemySystemNodes, EnemyMarkerClass);
}

// add or update the marker UI
void AAircraft::UpdateMarker(TArray<FTargetSystemNode*> TargetSystemNodes, TSubclassOf<ATargetSystemMarker> MarkerClass)
{
	if (!MarkerClass) {
		return;
	}

	if (TargetSystemNodes.Num() <= 0) {
		return;
	}

	for (int i = 0; i < TargetSystemNodes.Num(); i++)
	{
		auto TargetNode = TargetSystemNodes[i];
		auto Actor = TargetNode->Actor;

		// if the target is not active
		bool IsAlive = UHealthComponent::IsAlive(Actor);

		if (IsAlive)
		{
			if (TargetNode->Marker) // marker to follow the actor location
			{
				TargetNode->Marker->SetActorLocation(Actor->GetActorLocation());
			}
			else // otherwise create a marker if does not exist 
			{
				TargetNode->Marker = GetWorld()->SpawnActor<ATargetSystemMarker>(MarkerClass, Actor->GetActorLocation(), FRotator::ZeroRotator);
			}
		}
		else
		{
			// Destroy target marker
			if (TargetNode->Marker)
			{
				TargetNode->Marker->Destroy();
			}

			//TargetSystemNodes.RemoveAt(i);
		}

	}

}

bool AAircraft::DoesFriendlyNodeExists(AActor* TargetActor)
{
	for (FTargetSystemNode* node : FriendlyMarkerNodes)
	{
		if (node->Actor == TargetActor)
			return true;
	}

	return false;
}

bool AAircraft::DoesEnemyNodeExists(AActor* TargetActor)
{
	for (FTargetSystemNode* node : EnemySystemNodes)
	{
		if (node->Actor == TargetActor)
			return true;
	}

	return false;
}

void AAircraft::CreateThermalMatInstances()
{
	if (ThermalMaterials.Num() <= 0) {
		return;
	}

	for (int i = 0; i < ThermalMaterials.Num(); i++)
	{
		UMaterialInstanceDynamic* MaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), ThermalMaterials[i]);
		ThermalVisionPPComp->AddOrUpdateBlendable(MaterialInstance, 0.0f); // set all to invisible
		ThermalMaterialInstances.Add(MaterialInstance);
	}
}

void AAircraft::UpdateCurrentThermalVision(float InWeight)
{
	if (ThermalMaterialInstances.Num() <= 0) {
		return;
	}

	ThermalVisionPPComp->AddOrUpdateBlendable(ThermalMaterialInstances[CurrentThermalMatIndex], InWeight);
}

void AAircraft::ChangeThermalVision()
{
	if (ThermalMaterials.Num() <= 0) {
		return;
	}

	UpdateCurrentThermalVision(0.0f); // disable current material

	if (CurrentThermalMatIndex < ThermalMaterialInstances.Num() - 1)
	{
		CurrentThermalMatIndex++;
	}
	else
	{
		CurrentThermalMatIndex = 0;
	}

	ThermalToggleAudio->Play();

	UpdateCurrentThermalVision(1.0f);
}


void AAircraft::SetPlayerControl(APlayerController* OurPlayerController, bool EnableThermalPP, bool ShowOutline)
{
	OurPlayerController->SetViewTargetWithBlend(this, CameraSwitchDelay);
	UpdateWeaponView();
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetViewTargetWithBlend(CurrentWeaponObj, CameraSwitchDelay);

	ThermalVisionPPComp->bEnabled = EnableThermalPP;

	if (ThermalVisionPPComp->bEnabled)
	{
		UpdateCurrentThermalVision(1.0f);
	}

	AddUIWidget();

	// Play as soon as the player camera is set to aircraft camera
	GetWorldTimerManager().SetTimer(THandler_CameraSwitchDelay, this, &AAircraft::InitialContolSetup, CameraSwitchDelay, false);
}

void AAircraft::InitialContolSetup()
{
	if (InitialPilotSound)
	{
		PilotAudio->Sound = InitialPilotSound;
		PilotAudio->Play();
	}

	GetWorldTimerManager().ClearTimer(THandler_CameraSwitchDelay);

	float Delay = 5.0f;
	if (InitialPilotSound)
	{
		Delay = InitialPilotSound->Duration;
	}
	GetWorldTimerManager().SetTimer(THandler_RandomPiotSound, this, &AAircraft::PlayRandomPilotSound, Delay, true);
}

void AAircraft::PlayRandomPilotSound()
{
	if (RandomPilotSound == nullptr) {
		GetWorldTimerManager().ClearTimer(THandler_RandomPiotSound);
		return;
	}

	if (PilotAudio->IsPlaying()) {
		return;
	}

	PilotAudio->Sound = RandomPilotSound;
	PilotAudio->Play();

	// reset the timer based on the current sound being played as the new minimum delay
	GetWorldTimerManager().ClearTimer(THandler_RandomPiotSound);
	GetWorldTimerManager().SetTimer(THandler_RandomPiotSound, this, &AAircraft::PlayRandomPilotSound, RandomPilotSound->Duration, true);

}

void AAircraft::WaitForRapelling()
{
	if (CurrentAircraftMovement != EVehicleMovement::Rappel || OccupiedSeats.Num() <= 0) {
		return;
	}

	// to check if all have rapelled
	bool DoesSideGunnerRemain = false;

	CurveTimeline.Stop();

	// Create Rope objects
	if (RopeClass)
	{
		if (RopeLeft == nullptr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			RopeLeft = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			RopeLeft->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftRopeSocket);
		}

		if (RopeRight == nullptr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			RopeRight = GetWorld()->SpawnActor<ARope>(RopeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			RopeRight->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightRopeSocket);
		}

	}


	for (int i = 0; i < OccupiedSeats.Num(); i++)
	{
		if (OccupiedSeats[i].Role == EVehicleRole::SideGunner)
		{
			FAircraftSeating Passenger = OccupiedSeats[i];
			auto Character = Passenger.CharacterObj;

			if (Character)
			{
				DoesSideGunnerRemain = true;
				if (!Character->GetIsInVehicle())
				{
					OccupiedSeats.RemoveAt(i);
				}
				else
				{
					if (!Character->GetIsExitingVehicle())
					{
						if (Passenger.isRopeLeftSide)
						{
							if (!isLeftRappelOccupied)
							{
								Character->SetActorLocationAndRotation(Mesh->GetSocketLocation(LeftRopeSocket), FRotator::ZeroRotator);
								Character->SetIsExitingVehicle(true);
								isLeftRappelOccupied = true;
								DoesSideGunnerRemain = true;
							}
						}
						else
						{
							if (!isRightRappelOccupied)
							{
								Character->SetActorLocationAndRotation(Mesh->GetSocketLocation(RightRopeSocket), FRotator::ZeroRotator);
								Character->SetIsExitingVehicle(true);
								isRightRappelOccupied = true;
								DoesSideGunnerRemain = true;
							}
						}
					}
				}

			}
		}
	}


	if (!DoesSideGunnerRemain)
	{
		if (RopeLeft) {
			RopeLeft->DropRope();
		}

		if (RopeRight) {
			RopeRight->DropRope();
		}

		CurrentAircraftMovement = EVehicleMovement::MovingForward;

		CurveTimeline.Play();

	}
}

void AAircraft::UpdateOccupiedSeats()
{
	// check if passengers still alive
	for (int i = 0; i < OccupiedSeats.Num(); i++)
	{
		auto Character = OccupiedSeats[i].CharacterObj;

		if (Character == nullptr || Character->GetName() == "None")
		{
			OccupiedSeats.RemoveAt(i);
		}
		else if (OccupiedSeats[i].Role == EVehicleRole::SideGunner)
		{
			if (Character->GetHealthComp() && !Character->GetHealthComp()->IsAlive())
			{
				OccupiedSeats.RemoveAt(i);
			}
		}
	}
}

// Recursively destroy children actors
void AAircraft::DestroyChildActor(TArray<AActor*> ParentActor)
{
	for (int i = 0; i < ParentActor.Num(); i++)
	{
		AActor* ChildActor = ParentActor[i];

		// ignore the aircraft actor
		if (ChildActor == this) {
			continue;
		}

		TArray<AActor*> ChildAttachedActors;
		ChildActor->GetAttachedActors(ChildAttachedActors);
		DestroyChildActor(ChildAttachedActors);

		ChildActor->Destroy();
	}
}

void AAircraft::Destroyed()
{
	Super::Destroyed();

	RemoveUIWidget();

	if (HighlightCharacters)
	{
		ShowOutlines(false);
	}

	OnAircraftDestroy.Broadcast(this);


	// destroy markers
	if (FriendlyMarkerNodes.Num() > 0)
	{
		for (FTargetSystemNode* node : FriendlyMarkerNodes)
		{
			if (node->Marker)
			{
				node->Marker->Destroy();
			}
		}
	}

	if (FriendlyMarkerNodes.Num() > 0)
	{
		for (FTargetSystemNode* node : EnemySystemNodes)
		{
			if (node->Marker)
			{
				node->Marker->Destroy();
			}
		}
	}

	// Free up the path for another aircraft to use
	if (AircraftPath && AircraftPath->GetOccupiedVehicle() == this)
	{
		AircraftPath->SetOccupantVehicle(nullptr);
	}


	// destroy all attached actors to this aircraft
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	DestroyChildActor(AttachedActors);

}