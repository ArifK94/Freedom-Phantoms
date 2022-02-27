// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicles/VehicleBase.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"
#include "Props/TargetSystemMarker.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/WeaponBullet.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/ObjectPoolComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/VehiclePathFollowerComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/WidgetComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "AIController.h"

AVehicleBase::AVehicleBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetCollisionProfileName(TEXT("OverlapAll"));
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	RootComponent = CapsuleComponent;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetCollisionProfileName(TEXT("Vehicle"));
	MeshComponent->SetGenerateOverlapEvents(true);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->SetupAttachment(RootComponent);

	EyePointComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("EyePointComponent"));
	EyePointComponent->SetupAttachment(RootComponent);

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

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->Radius = 250.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true; // ignore self

	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(MeshComponent);

	// follow will contain any interior sounds so these sounds cannot be heard outside of the aircraft
	PilotAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PilotAudio"));
	PilotAudio->SetupAttachment(FollowCamera);

	ThermalVisionPPComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ThermalVisionPPComp"));
	ThermalVisionPPComp->SetupAttachment(FollowCamera);

	ThermalToggleAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ThermalToggleAudio"));
	ThermalToggleAudio->SetupAttachment(FollowCamera);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetRegenerateHealth(false);

	VehiclePathFollowerComponent = CreateDefaultSubobject<UVehiclePathFollowerComponent>(TEXT("VehiclePathFollowerComponent"));

	EngineSoundParamName = "Speed";

	RotationSpeed = 100000.0;

	CurrentWeaponIndex = 0;
	CameraSwitchDelay = .5f;
	ExplosionImpulse = 400.0f;
	ExplosionDamage = 30.0f;
	WheelRPM = 0.f;

	UseFollowCamNavigation = false;
	HighlightCharacters = false;
	SimulateExplosionPhysics = false;

	KillConfirmedParamName = "KillConfirmed";
	SingleKillIndex = 0;
	DoubleKillIndex = 1;
	MultiKillIndex = 2;
}

void AVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	ThermalVisionPPComp->bEnabled = false;

	HealthComponent->OnHealthChanged.AddDynamic(this, &AVehicleBase::OnHealthUpdate);

	CameraBoom->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraSocket);
	
	GetWorldTimerManager().SetTimer(THandler_Update, this, &AVehicleBase::Update, .2f, true);

	SpawnVehicleWeapons();
}

void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set the engine sound based on actor's velocity
	EngineAudio->SetFloatParameter(EngineSoundParamName, GetVelocity().Size());

	WheelRPM = (GetVelocity().Size() * 360.f) / 60.f;
}

void AVehicleBase::Update()
{

}

void AVehicleBase::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (EyePointComponent)
	{
		OutLocation = EyePointComponent->GetComponentLocation();
		OutRotation = EyePointComponent->GetComponentRotation();
	}
	else
	{
		OutLocation = MeshComponent->GetComponentLocation();
		OutRotation = MeshComponent->GetComponentRotation();
	}
}

void AVehicleBase::AddUIWidget()
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

void AVehicleBase::RemoveUIWidget()
{
	UWorld* World = GetWorld();

	if (!World) return;

	if (HUDWidget)
	{
		UWidgetLayoutLibrary::RemoveAllWidgets(World);
		HUDWidget = nullptr;
	}
}

void AVehicleBase::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!HealthComponent->IsAlive())
	{
		PrimaryActorTick.bCanEverTick = false;
		GetWorldTimerManager().ClearTimer(THandler_Update);

		// Remove all passengers
		for (int i = 0; i < VehicleSeatPtrList.Num(); i++)
		{
			auto Character = VehicleSeatPtrList[i]->Character;

			if (Character)
			{
				Character->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			}
		}

		// Destroy specified components
		for (int i = 0; i < DestroyableComponentList.Num(); i++)
		{
			auto Component = DestroyableComponentList[i];

			if (Component)
			{
				Component->DestroyComponent();
			}
		}

		for (int i = 0; i < VehicleWeaponPtrList.Num(); i++)
		{
			auto Weapon = VehicleWeaponPtrList[i]->Weapon;

			if (Weapon)
			{
				Weapon->Destroy();
			}
		}

		// Stop playing all audio components
		auto AudioActorComps = GetComponents();
		for (auto& Elem : AudioActorComps)
		{
			auto AudioComp = Cast<UAudioComponent>(Elem);
			if (AudioComp)
			{
				AudioComp->Sound = nullptr;
				AudioComp->Stop();
			}
		}


		if (SimulateExplosionPhysics)
		{
			MeshComponent->SetSimulatePhysics(true);


			// Boost upwards
			FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
			MeshComponent->AddImpulse(BoostIntensity, NAME_None, true);

		}

		// Play FX & change self material
		if (ExplosionEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		}

		// Play explosion sound
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, ExplosionAttenuation);
		}

		if (ExplosionMesh)
		{
			MeshComponent->SetSkeletalMesh(ExplosionMesh, false);
		}

		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();

		// Apply health damage
		ApplyExplosionDamage(GetActorLocation(), InHealthParameters);
	}
}


void AVehicleBase::OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters)
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

void AVehicleBase::SpawnVehicleWeapons()
{
	for (int i = 0; i < VehicleWeapons.Num(); i++)
	{
		auto VehicleWeapon = VehicleWeapons[i];
		auto MyOwner = GetOwner() ? GetOwner() : this;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = MyOwner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		auto Weapon = GetWorld()->SpawnActor<AMountedGun>(VehicleWeapon.WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Weapon)
		{
			Weapon->SetOwner(MyOwner);
			Weapon->SetAdjustBehindMG(false);
			Weapon->SetCanTraceInteraction(false);
			Weapon->SetCanExit(false);

			Weapon->SetPitchMin(VehicleWeapon.PitchMin);
			Weapon->SetPitchMax(VehicleWeapon.PitchMax);
			Weapon->SetYawMin(VehicleWeapon.YawMin);
			Weapon->SetYawMax(VehicleWeapon.YawMax);

			Weapon->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);
		}

		auto VehicleWeaponPtr = new FVehicleWeapon();
		VehicleWeaponPtr->Weapon = Weapon;
		VehicleWeaponPtrList.Add(VehicleWeaponPtr);
	}

	// We need to spawn the weapons before any characters in case any associated weapons have assigned for certain seats like turrets
	SpawnVehicleSeatings();
}

void AVehicleBase::SpawnVehicleSeatings()
{
	if (VehicleSeats.Num() <= 0) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int i = 0; i < VehicleSeats.Num(); i++)
	{
		auto VehicleSeat = VehicleSeats[i];

		if (!VehicleSeat.CharacterClass) {
			continue;
		}

		auto Character = GetWorld()->SpawnActor<ABaseCharacter>(VehicleSeat.CharacterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (Character)
		{
			Character->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleSeat.SeatingSocketName);

			// Set to use weapon, usually a mounted gun would be used
			if (VehicleSeat.AssociatedWeapon > -1)
			{
				auto AssociateIndex = VehicleSeat.AssociatedWeapon;
				auto MG = Cast<AMountedGun>(VehicleWeaponPtrList[VehicleSeat.AssociatedWeapon]->Weapon);
				if (MG) // if it's a mounted gun
				{
					ACombatCharacter* CombatCharacter = Cast<ACombatCharacter>(Character);
					CombatCharacter->SetMountedGun(MG);
					CombatCharacter->UseMountedGun();

					if (VehicleWeapons[AssociateIndex].AttachCharacterToWeapon)
					{
						CombatCharacter->AttachToActor(MG, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapons[AssociateIndex].WeaponAttachmentName);
					}

					// Ignore this vehicle when using target finder component
					UTargetFinderComponent::AddIgnoreActor(CombatCharacter, this);
				}
			}

			auto VehicleSeatPtr = new FVehicletSeating();
			VehicleSeatPtr->Character = Character;
			VehicleSeatPtr->OwningVehicle = this;
			Character->SetVehicleSeat(VehicleSeatPtr);
			VehicleSeatPtrList.Add(VehicleSeatPtr);
		}

	}
}

void AVehicleBase::ChangeThermalVision()
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

void AVehicleBase::UpdateCurrentThermalVision(float InWeight)
{
	if (ThermalMaterialInstances.Num() <= 0) {
		return;
	}

	ThermalVisionPPComp->AddOrUpdateBlendable(ThermalMaterialInstances[CurrentThermalMatIndex], InWeight);
}

void AVehicleBase::ShowOutlines(bool CanShow)
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


void AVehicleBase::UpdateWeaponView()
{
	// return previous characters back to AI posession
	if (VehicleSeatPtrList.Num() > 0 && CurrentWeaponIndex - 1 >= 0)
	{
		auto Seat = VehicleSeatPtrList[CurrentWeaponIndex - 1];
		Seat->Character->AutoPossessPlayer = EAutoReceiveInput::Disabled;
		if (Seat->Character->GetDefaultAIController())
		{
			Seat->Character->GetDefaultAIController()->Possess(Seat->Character);
		}
		Seat->Character->SetVehicleSeat(Seat); // so AI does not fall to the ground when repossessed
	}

	CurrentWeaponIndex = 0;// set the current weapon to first weapon by default

	VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->StopFire();
	VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->ChargeDown();

	// Player possessing is required if the character is posseseed by the AI controller
	if (VehicleSeatPtrList.Num() > 0)
	{
		auto Seat = VehicleSeatPtrList[CurrentWeaponIndex];
		Seat->Character->GetDefaultAIController()->UnPossess();
		Seat->Character->AutoPossessPlayer = EAutoReceiveInput::Player0;
		Seat->Character->EndAim();
	}

	if (UseFollowCamNavigation)
	{
		FollowCamera->AttachToComponent(VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->getMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->GetCameraPositionSocket());
		VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->GetFollowCamera()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FollowCamera->SetRelativeRotation(RotationInput);
		FollowCamera->SetFieldOfView(VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->GetFollowCamera()->FieldOfView);
	}
	else
	{
		FollowCamera->AttachToComponent(VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->GetFollowCamera(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	OnWeaponSwitch.Broadcast(this);
}

void AVehicleBase::SetPlayerControl(APlayerController* OurPlayerController, bool EnableThermalPP, bool ShowOutline)
{
	OurPlayerController->SetViewTargetWithBlend(this, CameraSwitchDelay);
	UpdateWeaponView();

	ThermalVisionPPComp->bEnabled = EnableThermalPP;

	if (ThermalVisionPPComp->bEnabled)
	{
		UpdateCurrentThermalVision(1.0f);
	}

	AddUIWidget();

	// Play as soon as the player camera is set to aircraft camera
	GetWorldTimerManager().SetTimer(THandler_CameraSwitchDelay, this, &AVehicleBase::InitialContolSetup, CameraSwitchDelay, false);
}

void AVehicleBase::InitialContolSetup()
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
	GetWorldTimerManager().SetTimer(THandler_RandomPiotSound, this, &AVehicleBase::PlayRandomPilotSound, Delay, true);
}

void AVehicleBase::PlayRandomPilotSound()
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
	GetWorldTimerManager().SetTimer(THandler_RandomPiotSound, this, &AVehicleBase::PlayRandomPilotSound, RandomPilotSound->Duration, true);

}

void AVehicleBase::SetTargetSystem()
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
void AVehicleBase::UpdateMarker(TArray<FTargetSystemNode*> TargetSystemNodes, TSubclassOf<ATargetSystemMarker> MarkerClass)
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
		}

	}

}

bool AVehicleBase::DoesFriendlyNodeExists(AActor* TargetActor)
{
	for (FTargetSystemNode* node : FriendlyMarkerNodes)
	{
		if (node->Actor == TargetActor)
			return true;
	}

	return false;
}

bool AVehicleBase::DoesEnemyNodeExists(AActor* TargetActor)
{
	for (FTargetSystemNode* node : EnemySystemNodes)
	{
		if (node->Actor == TargetActor)
			return true;
	}

	return false;
}


/// <summary>
/// Weapon control
/// </summary>
/// <param name="Val"></param>
void AVehicleBase::AddControllerPitchInput(float Val)
{
	if (UseFollowCamNavigation)
	{
		RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, VehicleWeaponPtrList[CurrentWeaponIndex]->PitchMin, VehicleWeaponPtrList[CurrentWeaponIndex]->PitchMax);
		RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

		FollowCamera->SetRelativeRotation(RotationInput);
	}
	else
	{
		VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->AddControllerPitchInput(Val);
		RotationInput = VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->GetRotationInput();
	}
}
void AVehicleBase::AddControllerYawInput(float Val)
{
	if (UseFollowCamNavigation)
	{
		RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, VehicleWeaponPtrList[CurrentWeaponIndex]->YawMin, VehicleWeaponPtrList[CurrentWeaponIndex]->YawMax);
		RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

		FollowCamera->SetRelativeRotation(RotationInput);
	}
	else
	{
		VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->AddControllerYawInput(Val);
		RotationInput = VehicleWeaponPtrList[CurrentWeaponIndex]->Weapon->GetRotationInput();
	}
}

/// <summary>
/// Third person view of the aircraft control
/// </summary>
/// <param name="Val"></param>
/// <param name="IsCameraRoam"></param>
void AVehicleBase::AddControllerPitchInput(float Val, bool IsCameraRoam)
{
	RotationInput.Pitch += Val;

	CameraBoom->SetWorldRotation(RotationInput);
}
void AVehicleBase::AddControllerYawInput(float Val, bool IsCameraRoam)
{
	RotationInput.Yaw += Val;

	CameraBoom->SetWorldRotation(RotationInput);
}

/// <summary>
/// Passenger Pitch Control
/// </summary>
/// <param name="Val"></param>
/// <param name="AircraftSeating"></param>
void AVehicleBase::AddControllerPitchInput(float Val, FVehicletSeating* VehicletSeating)
{
	RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, VehicletSeating->PitchMin, VehicletSeating->PitchMax);
	RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	VehicletSeating->Character->FollowCamera->SetRelativeRotation(RotationInput);
}

void AVehicleBase::AddControllerYawInput(float Val, FVehicletSeating* VehicletSeating)
{
	RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, VehicletSeating->YawMin, VehicletSeating->YawMax);
	RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	VehicletSeating->Character->FollowCamera->SetRelativeRotation(RotationInput);
}

void AVehicleBase::SetRotationInput(FRotator InRotation)
{
	//auto CurrentRotation = RotationInput;
	//auto TotalPitch = RotationInput.Pitch + InRotation.Pitch;
	//auto TotalYaw = RotationInput.Yaw + InRotation.Yaw;

	//ChangePitchValue(TotalYaw);

	//if (TotalPitch >= PitchMax) {
	//	TotalPitch = PitchMax;
	//}
	//else if (TotalPitch <= PitchMin) {
	//	TotalPitch = PitchMin;
	//}
	//else {
	//	TotalPitch = InRotation.Pitch;
	//}

	//if (TotalYaw >= YawMax) {
	//	TotalYaw = YawMax;
	//}
	//else if (TotalYaw <= YawMin) {
	//	TotalYaw = YawMin;
	//}
	//else {
	//	TotalYaw = InRotation.Yaw;
	//}

	//RotationInput.Pitch = TotalPitch;
	//RotationInput.Yaw = TotalYaw;

	RotationInput = InRotation;
}



void AVehicleBase::ApplyExplosionDamage(FVector ImpactPoint, FHealthParameters InHealthParams)
{
	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(RadialForceComp->Radius);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, ImpactPoint, ImpactPoint, FQuat::Identity, ECC_Visibility, MyColSphere);


	if (isHit)
	{
		// loop through TArray
		for (auto& Hit : OutHits)
		{
			AActor* DamagedActor = Hit.GetActor();
			if (DamagedActor)
			{
				UHealthComponent* DamagedHealthComponent = Cast<UHealthComponent>(DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

				if (DamagedHealthComponent && DamagedHealthComponent->IsAlive())
				{
					FHealthParameters HealthParameters;
					HealthParameters.Damage = ExplosionDamage;
					HealthParameters.DamagedActor = DamagedActor;
					HealthParameters.DamageCauser = InHealthParams.DamageCauser;
					HealthParameters.InstigatedBy = InHealthParams.InstigatedBy;
					HealthParameters.WeaponCauser = InHealthParams.WeaponCauser;
					HealthParameters.Bullet = InHealthParams.Bullet;
					HealthParameters.HitInfo = InHealthParams.HitInfo;
					HealthParameters.IsExplosive = true;
					DamagedHealthComponent->OnDamage(HealthParameters);
				}
			}
		}
	}
}

void AVehicleBase::AddComponentToDestroyList(UActorComponent* ActorComponent)
{
	if (ActorComponent)
	{
		DestroyableComponentList.Add(ActorComponent);
	}
}

// Recursively destroy children actors
void AVehicleBase::DestroyChildActor(TArray<AActor*> ParentActor)
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

void AVehicleBase::Destroyed()
{
	Super::Destroyed();

	OnVehicleDestroy.Broadcast(this);

	RemoveUIWidget();

	if (HighlightCharacters)
	{
		ShowOutlines(false);
	}



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

	VehiclePathFollowerComponent->ClearPath();


	// destroy all attached actors to this aircraft
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	DestroyChildActor(AttachedActors);

}