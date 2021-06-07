#include "Vehicles/Aircraft.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"
#include "Props/AircraftSplinePath.h"
#include "Accessories/Rope.h"
#include "Props/TargetSystemMarker.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"

#include "CustomComponents/HealthComponent.h"
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
	ThermalVisionPPComp->AttachToComponent(FollowCamera, FAttachmentTransformRules::KeepRelativeTransform);

	// follow will contain any interior sounds so these sounds cannot be heard outside of the aircraft
	PilotAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PilotAudio"));
	PilotAudio->AttachToComponent(FollowCamera, FAttachmentTransformRules::KeepRelativeTransform);

	ThermalToggleAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ThermalToggleAudio"));
	ThermalToggleAudio->AttachToComponent(FollowCamera, FAttachmentTransformRules::KeepRelativeTransform);


	WingRotationSpeed = 100000.0;

	PathFollowDuration = 10.0f;
	TotalLaps = 0;

	CurrentWeaponIndex = 0;

	CameraSwitchDelay = .5f;
}

void AAircraft::AddUIWidget()
{
	UWorld* World = GetWorld();

	if (!World) return;

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(World, HUDWidgetClass);

		if (HUDWidget)
		{
			UWidgetLayoutLibrary::RemoveAllWidgets(World);
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

	ThermalVisionPPComp->bEnabled = false;
	CreateThermalMatInstances();

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AAircraft::OnOverlapBegin);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AAircraft::OnOverlapBegin);

	FindPath();

	SpawnWeapon();
	
	EngineAudio->Play();

	SpawnPassenger();
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
		AAircraftSplinePath* CollidedPath = Cast<AAircraftSplinePath>(OtherActor);

		if (CollidedPath)
		{
			FVehicleSplinePoint CurrentSplinePoint = CollidedPath->GetVehicleSplinePoint(CollisionLocation);

			if (CurrentSplinePoint.PointIndex != -1)
			{
				CurrentAircraftMovement = CurrentSplinePoint.MovementType;
			}
		}
	}
}

void AAircraft::FindPath()
{
	// if already assigned a path then return
	if (AircraftPath != nullptr)
	{
		return;
	}

	TArray<AActor*> TargetActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), AircraftPathTagName, TargetActor);

	for (AActor* Actor : TargetActor)
	{
		AircraftPath = Cast<AAircraftSplinePath>(Actor);

		if (AircraftPath != nullptr)
		{

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
			break;
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
			OnDestroy();
		}
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
				Character->SetAircraftSeat(HeliSeat);

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
		AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(AircraftWeapon.Weapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Weapon)
		{
			if (AircraftWeapon.SetCamToMuzzle)
			{
				Weapon->SetComponentEyeViewPoint(FollowCamera);
			}

			Weapon->getMeshComp()->SetCollisionProfileName(TEXT("NoCollision")); // To allow line trace to go ignore this weapon, for AI sights for example
			Weapon->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AircraftWeapon.WeaponSocketName);
			Weapon->GetClipAudioComponent()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Weapon->GetShotAudioComponent()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			PilotAudio->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			ThermalToggleAudio->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			AMountedGun* MG = Cast<AMountedGun>(Weapon);

			// we do not want to adjust character behind MG as character will be spawning behind it
			// & remove line trace mechanic for it so only spawned characters can use it
			// & gunner cannot exit from gun
			if (MG)
			{
				MG->SetAdjustBehindMG(false);
				MG->SetCanTraceInteraction(false);
				MG->SetCanExit(false);

				MG->SetPitchMin(AircraftWeapon.PitchMin);
				MG->SetPitchMax(AircraftWeapon.PitchMax);
				MG->SetYawMin(AircraftWeapon.YawMin);
				MG->SetYawMax(AircraftWeapon.YawMax);
			}

			WeaponObjs.Add(Weapon);
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
}

void AAircraft::UpdateWeaponView()
{
	CurrentAircraftWeapon = AircraftWeapons[CurrentWeaponIndex]; // set the current weapon to first weapon by default
	CurrentWeaponObj = WeaponObjs[CurrentWeaponIndex];

	if (CurrentAircraftWeapon.SetCamToMuzzle)
	{
		FollowCamera->AttachToComponent(CurrentWeaponObj->getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeaponObj->GetMuzzleSocket()); // Attach to weapon muzzle
		FollowCamera->SetRelativeRotation(RotationInput); // set the follow camera to rotation input

		FollowCamera->SetFieldOfView(CurrentWeaponObj->GetZoomFOV());
	}


	OnUpdateWeaponUI.Broadcast(this);
}

void AAircraft::AddControllerPitchInput(float Val)
{
	RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, CurrentAircraftWeapon.PitchMin, CurrentAircraftWeapon.PitchMax);
	RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	FollowCamera->SetRelativeRotation(RotationInput);
}
void AAircraft::AddControllerYawInput(float Val)
{
	RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, CurrentAircraftWeapon.YawMin, CurrentAircraftWeapon.YawMax);
	RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	FollowCamera->SetRelativeRotation(RotationInput);
}

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

	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseCharacter::StaticClass(), Characters);

	// adding characters to targetting
	for (AActor* Actor : Characters)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(Actor);

		if (Character != nullptr)
		{
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Character->GetComponentByClass(UHealthComponent::StaticClass()));

			if (CurrentHealth)
			{
				bool isFriendly = UHealthComponent::IsFriendly(MyOwner, Character);

				if (CurrentHealth->IsAlive())
				{
					if (isFriendly)
					{
						if (!DoesFriendlyNodeExists(Character))
						{
							FTargetSystemNode* TargetNode = new FTargetSystemNode;
							TargetNode->Character = Character;
							TargetNode->Marker = nullptr;
							FriendlyMarkerNodes.Add(TargetNode);
						}
					}
					else
					{
						if (!DoesEnemyNodeExists(Character))
						{
							FTargetSystemNode* TargetNode = new FTargetSystemNode;
							TargetNode->Character = Character;
							TargetNode->Marker = nullptr;
							EnemySystemNodes.Add(TargetNode);
						}
					}
				}
			}
		}
	}

	UpdateMarker(FriendlyMarkerNodes, FriendlyMarkerClass);
	UpdateMarker(EnemySystemNodes, EnemyMarkerClass);
}

// add or update the marker UI
void AAircraft::UpdateMarker(TArray<FTargetSystemNode*> TargetSystemNodes, TSubclassOf<ATargetSystemMarker> MarkerClass)
{
	if (MarkerClass == nullptr) {
		return;
	}

	if (TargetSystemNodes.Num() <= 0) {
		return;
	}

	for (int i = 0; i < TargetSystemNodes.Num(); i++)
	{
		FTargetSystemNode* TargetNode = TargetSystemNodes[i];

		// remove dead characters from the targetting system
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(TargetNode->Character->GetComponentByClass(UHealthComponent::StaticClass()));

		if (CurrentHealth)
		{
			if (CurrentHealth->IsAlive())
			{
				// add or update target marker based on character location
				FActorSpawnParameters SpawnParams;

				if (TargetNode->Marker == nullptr)
				{
					TargetNode->Marker = GetWorld()->SpawnActor<ATargetSystemMarker>(MarkerClass, TargetNode->Character->GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

				}
				else
				{
					TargetNode->Marker->SetActorLocation(TargetNode->Character->GetActorLocation());
				}
			}
			else
			{
				if (TargetNode->Marker) {
					TargetNode->Marker->GetMarkerComponent()->SetHiddenInGame(true);
				}
			}
		}
	}

}

bool AAircraft::DoesFriendlyNodeExists(AActor* TargetActor)
{
	for (FTargetSystemNode* node : FriendlyMarkerNodes)
	{
		if (node->Character == TargetActor)
			return true;
	}

	return false;
}

bool AAircraft::DoesEnemyNodeExists(AActor* TargetActor)
{
	for (FTargetSystemNode* node : EnemySystemNodes)
	{
		if (node->Character == TargetActor)
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

	ThermalVisionPPComp->bEnabled = EnableThermalPP;

	if (EnableThermalPP)
	{
		UpdateCurrentThermalVision(1.0f);
	}

	ShowOutlines(ShowOutline);
	AddUIWidget();

	GetWorldTimerManager().SetTimer(THandler_CameraSwitchDelay, this, &AAircraft::PlayPilotSound, CameraSwitchDelay, false);
}

void AAircraft::RemovePlayerControl()
{

}

void AAircraft::PlayPilotSound()
{
	PilotAudio->Play();
	GetWorldTimerManager().ClearTimer(THandler_CameraSwitchDelay);
}

void AAircraft::WaitForRapelling()
{
	if (CurrentAircraftMovement == EAircraftMovement::Rappel && OccupiedSeats.Num() > 0)
	{
		CurveTimeline.Stop();

		if (RopeClass != nullptr)
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
			if (OccupiedSeats[i].Role == EAircraftRole::SideGunner)
			{
				FAircraftSeating Passenger = OccupiedSeats[i];

				if (!Passenger.CharacterObj->GetIsInAircraft()) {
					OccupiedSeats.RemoveAt(i);
				}
				else
				{
					if (!Passenger.CharacterObj->IsRepellingDown())
					{
						if (Passenger.isRopeLeftSide)
						{
							if (!isLeftRappelOccupied)
							{
								Passenger.CharacterObj->SetActorLocationAndRotation(Mesh->GetSocketLocation(LeftRopeSocket), FRotator::ZeroRotator);
								Passenger.CharacterObj->SetIsRepellingDown(true);
								isLeftRappelOccupied = true;
							}
						}
						else
						{
							if (!isRightRappelOccupied)
							{
								Passenger.CharacterObj->SetActorLocationAndRotation(Mesh->GetSocketLocation(RightRopeSocket), FRotator::ZeroRotator);
								Passenger.CharacterObj->SetIsRepellingDown(true);
								isRightRappelOccupied = true;
							}
						}
					}

				}
			}
		}
	}

	if (OccupiedSeats.Num() <= 0)
	{
		if (RopeLeft) {
			RopeLeft->DropRope();
		}

		if (RopeRight) {
			RopeRight->DropRope();
		}

		CurveTimeline.Play();

		CurrentAircraftMovement = EAircraftMovement::MovingForward;
	}
}

void AAircraft::UpdateOccupiedSeats()
{
	// check if passengers still alive
	for (int i = 0; i < OccupiedSeats.Num(); i++)
	{
		if (OccupiedSeats[i].Role == EAircraftRole::SideGunner)
		{
			FAircraftSeating Passenger = OccupiedSeats[i];
			if (Passenger.CharacterObj) {

				UHealthComponent* CurrentHealth = Cast<UHealthComponent>(Passenger.CharacterObj->GetComponentByClass(UHealthComponent::StaticClass()));

				if (!CurrentHealth->IsAlive())
				{
					OccupiedSeats.RemoveAt(i);
				}
			}
		}
	}
}


void AAircraft::OnDestroy()
{
	RemoveUIWidget();
	ShowOutlines(false);

	OnAircraftDestroy.Broadcast(this);


	// destroy all attached actors to this aircraft
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* ChildActor : AttachedActors)
	{
		ChildActor->Destroy();
	}

	Destroy();

}