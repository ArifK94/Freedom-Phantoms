// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicles/VehicleBase.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"
#include "Props/TargetSystemMarker.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/Projectile.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/ObjectPoolComponent.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/VehiclePathFollowerComponent.h"
#include "CustomComponents/OptimizerComponent.h"
#include "Managers/DatatableManager.h"
#include "Services/SharedService.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
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
#include "Niagara/Public/NiagaraFunctionLibrary.h"

AVehicleBase::AVehicleBase()
{
	PrimaryActorTick.bCanEverTick = true;
	CanActorTick = true;

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
	MeshComponent->SetCanEverAffectNavigation(true);
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	FrontCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontCollider"));
	FrontCollider->SetCollisionProfileName(TEXT("OverlapAll"));
	FrontCollider->CanCharacterStepUpOn = ECB_No;
	FrontCollider->SetupAttachment(MeshComponent);

	FrontKillZoneComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontKillZoneComponent"));
	FrontKillZoneComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	FrontKillZoneComponent->CanCharacterStepUpOn = ECB_No;
	FrontKillZoneComponent->SetupAttachment(MeshComponent);

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

	IncomingThreatAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("IncomingThreatAudio"));
	IncomingThreatAudio->SetupAttachment(FollowCamera);
	IncomingThreatAudio->SetAutoActivate(false);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetRegenerateHealth(false);

	VehiclePathFollowerComponent = CreateDefaultSubobject<UVehiclePathFollowerComponent>(TEXT("VehiclePathFollowerComponent"));

	OptimizerComponent = CreateDefaultSubobject<UOptimizerComponent>(TEXT("OptimizerComponent"));

	// Vehicle path follower should not be optimized as the lag can cause the vehicle to miss its path trigger points.
	OptimizerComponent->IgnoredComponentClasses.Add(UVehiclePathFollowerComponent::StaticClass());
	
	EngineSoundParamName = "speed";

	RotationSpeed = 100000.0;

	CurrentWeaponIndex = 0;
	CameraSwitchDelay = .5f;
	ExplosionImpulse = 400.0f;
	ExplosionDamage = 30.0f;
	WheelRPM = 0.f;

	UseFollowCamNavigation = false;
	HighlightCharacters = false;
	SimulateExplosionPhysics = false;
	CheckFrontCollision = true;
	IsStationary = false;
	HasNoPlayerInput = false;
	MeshComponentTickEnabled = true;
	DestroyOnDeath = false;

	KillConfirmedParamName = "KillConfirmed";
	SingleKillIndex = 0;
	DoubleKillIndex = 1;
	MultiKillIndex = 2;

	SurfaceImpactRowName = "Vehicle_Destruction";

	// ignore other vehicles by default.
	CollisionClassFilters.Add(AVehicleBase::StaticClass());
}

void AVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	SurfaceImpactSet = UDatatableManager::RetrieveSurfaceImpactSet(GetWorld(), SurfaceImpactRowName);

	ThermalVisionPPComp->bEnabled = false;

	FrontKillZoneComponent->OnComponentBeginOverlap.AddDynamic(this, &AVehicleBase::OnVehicleBeginOverlap);

	HealthComponent->OnHealthChanged.AddDynamic(this, &AVehicleBase::OnHealthUpdate);

	CameraBoom->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraSocket);

	SpawnVehicleWeapons();

	CreateThermalMatInstances();

	SetActorTickEnabled(CanActorTick);

	if (CanActorTick)
	{
		GetWorldTimerManager().SetTimer(THandler_Update, this, &AVehicleBase::TimerTick, .2f, true, 2.f);
	}

	OptimizeComponents();
}

void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Set the engine sound based on actor's velocity
	if (EngineAudio)
	{
		EngineAudio->SetFloatParameter(EngineSoundParamName, GetVelocity().Size());
	}

	WheelRPM = (GetVelocity().Size() * 360.f) / 60.f;

	CurrentRotationSpeed = DeltaTime * RotationSpeed;

	if (ShowTargetSystem)
	{
		SetTargetSystem();
	}

	// stop following path if front collider has detetced something.
	if (VehiclePathFollowerComponent)
	{
		if (ShouldStopVehicle())
		{
			VehiclePathFollowerComponent->Stop();
		}
		else
		{
			VehiclePathFollowerComponent->ResumePath();
		}
	}

}

void AVehicleBase::TimerTick()
{
	UpdatePassengerSeats();
}

void AVehicleBase::UpdatePassengerSeats()
{
	if (VehicleSeats.IsEmpty()) {
		return;
	}

	auto PassengerList = VehicleSeats;

	// Remove passengers if not alive
	for (int i = PassengerList.Num() - 1; i >= 0; i--)
	{
		auto Character = PassengerList[i].Character;

		if (!UHealthComponent::IsActorAlive(Character) || Character->GetVehicletSeat().OwningVehicle != this)
		{
			VehicleSeats.RemoveAt(i);
		}
	}
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

bool AVehicleBase::ShouldStopVehicle()
{
	return (CheckFrontCollision && IsFrontCollisionFound()) || (VehiclePathFollowerComponent && VehiclePathFollowerComponent->ShouldStopVehicle());
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

void AVehicleBase::OnVehicleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr || OtherActor == this) {
		return;
	}

	// apply damage to only characters.
	if (!OtherActor->IsA(ACharacter::StaticClass())) {
		return;
	}

	// ignore vehicle damage is not moving or moving slowly.
	if (GetVelocity().Size() <= 20.f) {
		return;
	}

	// ignore other actor if not in sight.
	if (!USharedService::CanSeeTarget(GetWorld(), GetActorLocation(), OtherActor, this)) {
		return;
	}

	float DamageReduction = 5.f;

	FVector Location = OtherActor->GetActorLocation();
	Location += OtherActor->GetActorForwardVector() + 50;
	OtherActor->SetActorLocation(Location);

	FHealthParameters HealthParameters;
	HealthParameters.DamagedActor = OtherActor;
	HealthParameters.DamageCauser = this;
	HealthParameters.InstigatedBy = GetInstigatorController();
	HealthParameters.Damage = GetVelocity().Size() / DamageReduction;
	HealthParameters.CanDamageFriendlies = true;
	UHealthComponent::ApplyDamage(HealthParameters);
}

void AVehicleBase::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!HealthComponent->IsAlive())
	{
		SetActorTickEnabled(false);
		GetWorldTimerManager().ClearTimer(THandler_Update);

		if (FrontKillZoneComponent)
		{
			FrontKillZoneComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AVehicleBase::OnVehicleBeginOverlap);
		}

		if (VehiclePathFollowerComponent) 
		{
			VehiclePathFollowerComponent->Stop();
			VehiclePathFollowerComponent->ClearPath();
		}

		if (EngineAudio)
		{
			EngineAudio->Stop();
		}

		// Remove all passengers
		for (int i = 0; i < VehicleSeats.Num(); i++)
		{
			auto Character = VehicleSeats[i].Character;

			if (Character)
			{
				Character->SetDefaultState();
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

		for (int i = 0; i < VehicleWeapons.Num(); i++)
		{
			auto Weapon = VehicleWeapons[i].Weapon;

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

		// Play FX
		if (SurfaceImpactSet)
		{
			if (SurfaceImpactSet->NiagaraEffect)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SurfaceImpactSet->NiagaraEffect, GetActorLocation(), SurfaceImpactSet->VFXOffset.GetRotation().Rotator());
			}

			// Play explosion sound
			if (SurfaceImpactSet->Sound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceImpactSet->Sound, GetActorLocation(), 1.0f, 1.0f, 0.0f, ExplosionAttenuation);
			}
		}


		if (ExplosionMesh)
		{
			MeshComponent->SetSkeletalMesh(ExplosionMesh, false);
			MeshComponent->AddWorldTransform(ExplosionMeshTransformOffset);
		}

		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();

		// Apply health damage
		ApplyExplosionDamage(GetActorLocation(), InHealthParameters);

		if (DestroyOnDeath)
		{
			// hide actor for now since some other actors may rely on this actor's reference
			SetActorHiddenInGame(true);

			SetActorEnableCollision(false);

			// then set to destroy the actor after x seconds assuming x is enough time for this actor to no longer be referenced.
			SetLifeSpan(5.f);
		}
	}
}


void AVehicleBase::OnWeaponKillConfirm(FProjectileImpactParameters ProjectileImpactParameters)
{
	if (ProjectileImpactParameters.KillCount <= 0) {
		return;
	}

	if (PilotAudio)
	{
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

		VehicleWeapons[i].Weapon = GetWorld()->SpawnActor<AMountedGun>(VehicleWeapon.WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (VehicleWeapons[i].Weapon)
		{
			VehicleWeapons[i].Weapon->SetOwner(MyOwner);
			VehicleWeapons[i].Weapon->SetAdjustBehindMG(false);
			VehicleWeapons[i].Weapon->SetCanTraceInteraction(false);
			VehicleWeapons[i].Weapon->SetCanExit(false);

			VehicleWeapons[i].Weapon->SetPitchMin(VehicleWeapon.PitchMin);
			VehicleWeapons[i].Weapon->SetPitchMax(VehicleWeapon.PitchMax);
			VehicleWeapons[i].Weapon->SetYawMin(VehicleWeapon.YawMin);
			VehicleWeapons[i].Weapon->SetYawMax(VehicleWeapon.YawMax);

			VehicleWeapons[i].Weapon->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);

			VehicleWeapons[i].Weapon->OnKillConfirmed.AddDynamic(this, &AVehicleBase::OnWeaponKillConfirm);
		}
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

		if (!Character) {
			continue;
		}

		Character->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleSeat.SeatingSocketName);

		// Set to use weapon, usually a mounted gun would be used
		if (VehicleSeat.AssociatedWeapon > -1)
		{
			auto AssociateIndex = VehicleSeat.AssociatedWeapon;
			auto MG = Cast<AMountedGun>(VehicleWeapons[VehicleSeat.AssociatedWeapon].Weapon);
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

		VehicleSeat.OwningVehicle = this;
		VehicleSeat.Character = Character;

		Character->SetVehicleSeat(VehicleSeat);

		VehicleSeats[i] = VehicleSeat;
	}
}

void AVehicleBase::RemovePassenger(int Index)
{
	if (VehicleSeats.Num() > 0 && Index < VehicleSeats.Num())
	{
		VehicleSeats.RemoveAt(Index);
	}
}

void AVehicleBase::CreateThermalMatInstances()
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

void AVehicleBase::ChangeThermalVision()
{
	if (ThermalMaterials.Num() <= 0) {
		return;
	}

	UpdateCurrentThermalVision(0.0f); // disable current material

	if (CurrentThermalMatIndex >= ThermalMaterialInstances.Num() - 1)
	{
		CurrentThermalMatIndex = 0;
	}
	else
	{
		CurrentThermalMatIndex++;
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
	if (VehicleWeapons.Num() <= 0) {
		return;
	}

	// return previous characters back to AI posession
	if (VehicleSeats.Num() > 0 && CurrentWeaponIndex - 1 >= 0)
	{
		auto Seat = VehicleSeats[CurrentWeaponIndex - 1];

		if (UHealthComponent::IsActorAlive(Seat.Character)) {

			Seat.Character->AutoPossessPlayer = EAutoReceiveInput::Disabled;
			if (Seat.Character->GetDefaultAIController())
			{
				Seat.Character->GetDefaultAIController()->Possess(Seat.Character);
			}
			Seat.Character->SetVehicleSeat(VehicleSeats[CurrentWeaponIndex - 1]); // so AI does not fall to the ground when repossessed
		}
	}

	CurrentWeapon = VehicleWeapons[CurrentWeaponIndex].Weapon;

	auto MyOwner = GetOwner() ? GetOwner() : this;
	CurrentWeapon->SetOwner(MyOwner);

	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetViewTargetWithBlend(CurrentWeapon);


	if (VehicleSeats.Num() > 0)
	{
		auto Index = CurrentWeaponIndex >= VehicleSeats.Num() ? 0 : CurrentWeaponIndex;

		// Player possessing is required if the character is posseseed by the AI controller
		auto Seat = VehicleSeats[Index];

		if (UHealthComponent::IsActorAlive(Seat.Character)) {

			if (Seat.Character->GetDefaultAIController()) {
				Seat.Character->GetDefaultAIController()->UnPossess();
			}

			Seat.Character->AutoPossessPlayer = EAutoReceiveInput::Player0;
		}
	}



	if (UseFollowCamNavigation)
	{
		FollowCamera->AttachToComponent(CurrentWeapon->GetMeshComp(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, CurrentWeapon->GetCameraPositionSocket());
		CurrentWeapon->GetFollowCamera()->AttachToComponent(FollowCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		FollowCamera->SetRelativeRotation(RotationInput);
		FollowCamera->SetFieldOfView(CurrentWeapon->GetFollowCamera()->FieldOfView);
	}
	else
	{
		FollowCamera->AttachToComponent(CurrentWeapon->GetFollowCamera(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	OnWeaponSwitch.Broadcast(this);
}

void AVehicleBase::ChangeWeapon()
{
	if (VehicleWeapons.Num() <= 0) {
		return;
	}

	// increment the index if current index is less than the array of weapons
	// otherwise go back to the first index
	if (CurrentWeaponIndex >= VehicleWeapons.Num() - 1)
	{
		CurrentWeaponIndex = 0;
	}
	else
	{
		CurrentWeaponIndex++;
	}

	if (CurrentWeapon) {
		CurrentWeapon->StopFire(); // stop firing current weapon before switching to another
		EndAim();
	}

	UpdateWeaponView();
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetViewTargetWithBlend(CurrentWeapon);
}

void AVehicleBase::BeginAim()
{
	CurrentWeapon->SetIsAiming(true);
}

void AVehicleBase::EndAim()
{
	CurrentWeapon->SetIsAiming(false);
}

void AVehicleBase::SetPlayerControl(APlayerController* OutPlayerController, bool EnableThermalPP, bool ShowOutline)
{
	UserController = OutPlayerController;

	ShowTargetSystem = true;

	OutPlayerController->SetViewTargetWithBlend(this, CameraSwitchDelay);
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

void AVehicleBase::RemovePlayerControl()
{
	UserController = nullptr;

	ShowTargetSystem = false;

	GetWorldTimerManager().ClearTimer(THandler_CameraSwitchDelay);

	ThermalVisionPPComp->bEnabled = false;

	if (ThermalVisionPPComp->bEnabled)
	{
		UpdateCurrentThermalVision(1.0f);
	}

	RemoveTargetSystem();
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
				if (!DoesNodeExist(FriendlyMarkerNodes, Actor))
				{
					auto TargetNode = FTargetSystemNode();
					TargetNode.Actor = Actor;
					TargetNode.Marker = GetWorld()->SpawnActor<ATargetSystemMarker>(FriendlyMarkerClass, TargetNode.Actor->GetActorLocation(), FRotator::ZeroRotator);
					FriendlyMarkerNodes.Add(TargetNode);
				}
			}
			else
			{
				if (!DoesNodeExist(EnemySystemNodes, Actor))
				{
					auto TargetNode = FTargetSystemNode();
					TargetNode.Actor = Actor;
					TargetNode.Marker = GetWorld()->SpawnActor<ATargetSystemMarker>(EnemyMarkerClass, TargetNode.Actor->GetActorLocation(), FRotator::ZeroRotator);
					EnemySystemNodes.Add(TargetNode);
				}
			}
		}
	}

	if (HighlightCharacters)
	{
		ShowOutlines(true);
	}

	UpdateMarker(FriendlyMarkerNodes);
	UpdateMarker(EnemySystemNodes);
}

bool AVehicleBase::IsFrontCollisionFound()
{
	if (!FrontCollider) {
		return false;
	}

	TArray<AActor*> OutOverlappingActors;

	// Get all overlapped actors based that have team faction component attached
	FrontCollider->GetOverlappingActors(OutOverlappingActors, AActor::StaticClass());

	for (AActor* Actor : OutOverlappingActors)
	{
		if (Actor == this) {
			continue;
		}
		for (auto collisionClass : CollisionClassFilters)
		{
			if (Actor->IsA(collisionClass)) {

				// ignore destroyed vehicles or dead actors to avoid having vehicles stopping from passing through.
				if (!UHealthComponent::IsActorAlive(Actor)) {
					return false;
				}

				return true;
			}
		}
	}

	return false;
}

// add or update the marker UI
void AVehicleBase::UpdateMarker(TArray<FTargetSystemNode> TargetSystemNodes)
{
	if (TargetSystemNodes.Num() <= 0) {
		return;
	}

	for (int i = TargetSystemNodes.Num() - 1; i >= 0; i--)
	{
		// if the target is alive
		if (UHealthComponent::IsActorAlive(TargetSystemNodes[i].Actor)) {

			// marker to follow the actor location
			if (TargetSystemNodes[i].Marker) {
				TargetSystemNodes[i].Marker->SetActorLocation(TargetSystemNodes[i].Actor->GetActorLocation());
			}
		}
		else {
			// Destroy target marker
			if (TargetSystemNodes[i].Marker) {
				TargetSystemNodes[i].Marker->Destroy();
			}

			TargetSystemNodes.RemoveAt(i);
		}
	}
}

bool AVehicleBase::DoesNodeExist(TArray<FTargetSystemNode> TargetSystemNodes, AActor* TargetActor)
{
	if (TargetSystemNodes.Num() <= 0) {
		return false;
	}

	for (auto node : TargetSystemNodes)
	{
		if (node.Actor == TargetActor) {
			return true;
		}
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
		RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, VehicleWeapons[CurrentWeaponIndex].PitchMin, VehicleWeapons[CurrentWeaponIndex].PitchMax);
		RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

		FollowCamera->SetRelativeRotation(RotationInput);
	}
	else
	{
		CurrentWeapon->AddControllerPitchInput(Val);
		RotationInput = CurrentWeapon->GetRotationInput();
	}
}
void AVehicleBase::AddControllerYawInput(float Val)
{
	if (UseFollowCamNavigation)
	{
		RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, VehicleWeapons[CurrentWeaponIndex].YawMin, VehicleWeapons[CurrentWeaponIndex].YawMax);
		RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

		FollowCamera->SetRelativeRotation(RotationInput);
	}
	else
	{
		CurrentWeapon->AddControllerYawInput(Val);
		RotationInput = CurrentWeapon->GetRotationInput();
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
void AVehicleBase::AddControllerPitchInput(float Val, FVehicletSeating VehicletSeating)
{
	RotationInput.Pitch = FMath::ClampAngle(RotationInput.Pitch + Val, VehicletSeating.PitchMin, VehicletSeating.PitchMax);
	RotationInput.Pitch = FRotator::ClampAxis(RotationInput.Pitch);

	VehicletSeating.Character->FollowCamera->SetRelativeRotation(RotationInput);
}

void AVehicleBase::AddControllerYawInput(float Val, FVehicletSeating VehicletSeating)
{
	RotationInput.Yaw = FMath::ClampAngle(RotationInput.Yaw + Val, VehicletSeating.YawMin, VehicletSeating.YawMax);
	RotationInput.Yaw = FRotator::ClampAxis(RotationInput.Yaw);

	VehicletSeating.Character->FollowCamera->SetRelativeRotation(RotationInput);
}

void AVehicleBase::SetRotationInput(FRotator InRotation)
{
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
					HealthParameters.Projectile = InHealthParams.Projectile;
					HealthParameters.HitInfo = InHealthParams.HitInfo;
					HealthParameters.IsExplosive = true;
					DamagedHealthComponent->OnDamage(HealthParameters);
				}
			}
		}
	}
}

void AVehicleBase::RemoveTargetSystem()
{
	RemoveUIWidget();

	if (HighlightCharacters) {
		ShowOutlines(false);
	}

	// destroy markers
	if (FriendlyMarkerNodes.Num() > 0) {
		for (auto node : FriendlyMarkerNodes)
		{
			if (node.Marker) {
				node.Marker->Destroy();
			}
		}
	}

	if (FriendlyMarkerNodes.Num() > 0) {
		for (auto node : EnemySystemNodes)
		{
			if (node.Marker) {
				node.Marker->Destroy();
			}
		}
	}

}

void AVehicleBase::OnMissileIncoming_Implementation(AProjectile* Missile)
{
	if (Missile)
	{
		IncomingMissiles.AddUnique(Missile);

		if (IncomingThreatAudio->GetSound() && !IncomingThreatAudio->IsPlaying())
		{
			IncomingThreatAudio->Play();
		}


		FIncomingThreatParameters IncomingThreatParams;
		IncomingThreatParams.Missile = Missile;
		IncomingThreatParams.bThreatDetected = true;
		IncomingThreatParams.IncomingMissileCount = IncomingMissiles.Num();
		OnIncomingThreatUpdate.Broadcast(IncomingThreatParams);
	}
}

void AVehicleBase::OnMissileDestroyed_Implementation(AProjectile* Missile)
{
	if (IncomingMissiles.Contains(Missile))
	{
		IncomingMissiles.Remove(Missile);
	}

	// If no missiles incoming
	if (IncomingMissiles.IsEmpty())
	{
		IncomingThreatAudio->Stop();
	}

	FIncomingThreatParameters IncomingThreatParams;
	IncomingThreatParams.bThreatDetected = !IncomingMissiles.IsEmpty();
	IncomingThreatParams.IncomingMissileCount = IncomingMissiles.Num();
	OnIncomingThreatUpdate.Broadcast(IncomingThreatParams);
}

void AVehicleBase::AddComponentToDestroyList(UActorComponent* ActorComponent)
{
	if (ActorComponent) {
		DestroyableComponentList.Add(ActorComponent);
	}
}

void AVehicleBase::OptimizeComponents()
{
	if (IsStationary)
	{
		USharedService::DestroyActorComponent(FrontCollider);
		USharedService::DestroyActorComponent(FrontKillZoneComponent);
		USharedService::DestroyActorComponent(FrontCollider);
		USharedService::DestroyActorComponent(CameraBoom);
		USharedService::DestroyActorComponent(FollowCamera);
		USharedService::DestroyActorComponent(EngineAudio);
		USharedService::DestroyActorComponent(VehiclePathFollowerComponent);
		USharedService::DestroyActorComponent(OptimizerComponent);

	}

	if (HasNoPlayerInput)
	{
		USharedService::DestroyActorComponent(PilotAudio);
		USharedService::DestroyActorComponent(ThermalVisionPPComp);
		USharedService::DestroyActorComponent(ThermalToggleAudio);
	}

	MeshComponent->SetComponentTickEnabled(MeshComponentTickEnabled);
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

	RemoveTargetSystem();

	// destroy all attached actors to this aircraft
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	DestroyChildActor(AttachedActors);

	OnVehicleDestroy.Broadcast(this);
}