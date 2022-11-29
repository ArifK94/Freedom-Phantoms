// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/LandVehicle.h"
#include "Weapons/Weapon.h"
#include "Weapons/MountedGun.h"
#include "Weapons/Projectile.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TargetFinderComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"

ALandVehicle::ALandVehicle()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionProfileName(TEXT("Vehicle"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	MeshComp->SetGenerateOverlapEvents(true);
	MeshComp->SetNotifyRigidBodyCollision(true);
	RootComponent = MeshComp;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent")); // capsule component is used for line trace
	CapsuleComponent->SetupAttachment(MeshComp);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn); // line trace for objects only works with pawn object type for some reason,


	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(MeshComp);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->TargetArmLength = 0.f;
	CameraBoom->SocketOffset.Set(.0f, .0f, .0f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true; // ignore self

	ExplosionImpulse = 400.0f;
	ExplosionDamage = 30.0f;

	SimulateExplosionPhysics = false;

	EngineSoundParamName = "Speed";
}

void ALandVehicle::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (FollowCamera)
	{
		OutLocation = FollowCamera->GetComponentLocation();
		OutRotation = FollowCamera->GetComponentRotation();
	}
	else
	{
		OutLocation = MeshComp->GetComponentLocation();
		OutRotation = MeshComp->GetComponentRotation();
	}
}

void ALandVehicle::AddComponentToDestroyList(UActorComponent* ActorComponent)
{
	if (ActorComponent)
	{
		ActorComponentsToDestroy.Add(ActorComponent);
	}
}


void ALandVehicle::BeginPlay()
{
	Super::BeginPlay();

	CameraBoom->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CameraSocket);

	HealthComp->OnHealthChanged.AddDynamic(this, &ALandVehicle::OnHealthUpdate);

	GetWorldTimerManager().SetTimer(THandler_Update, this, &ALandVehicle::Update, .2f, true);

	SpawnVehicleWeapons();
}

void ALandVehicle::Update()
{
	// Set the engine sound based on actor velocity
	EngineAudio->SetFloatParameter(EngineSoundParamName, GetVelocity().Size());
}

void ALandVehicle::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (IsDestroyed) {
		return;
	}

	if (!HealthComp->IsAlive())
	{
		PrimaryActorTick.bCanEverTick = false;

		// Remove all passengers
		for (int i = 0; i < VehicleCharactersPtr.Num(); i++)
		{
			auto Character = VehicleCharactersPtr[i];

			if (Character)
			{
				Character->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			}
		}

		// Destroy specified components
		for (int i = 0; i < ActorComponentsToDestroy.Num(); i++)
		{
			auto Component = ActorComponentsToDestroy[i];

			if (Component)
			{
				Component->DestroyComponent();
			}
		}

		for (int i = 0; i < VehicleWeaponsPtr.Num(); i++)
		{
			auto Weapon = VehicleWeaponsPtr[i];

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

		// Explode!
		IsDestroyed = true;

		if (SimulateExplosionPhysics)
		{
			MeshComp->SetSimulatePhysics(true);


			// Boost upwards
			FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
			MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

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
			MeshComp->SetSkeletalMesh(ExplosionMesh, false);
		}

		// Blast away nearby physics actors
		RadialForceComp->FireImpulse();

		// Apply health damage
		ApplyExplosionDamage(GetActorLocation(), InHealthParameters);
	}
}

void ALandVehicle::SpawnVehicleSeatings()
{
	//if (VehicletSeating.Num() <= 0) {
	//	return;
	//}

	//FActorSpawnParameters SpawnParams;
	//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//for (int i = 0; i < VehicletSeating.Num(); i++)
	//{
	//	auto VehicleSeat = VehicletSeating[i];
	//	VehicleSeat.OwningVehicle = this;

	//	if (VehicleSeat.Character)
	//	{
	//		VehicleSeat.Character = GetWorld()->SpawnActor<ABaseCharacter>(VehicleSeat.Character, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	//		if (VehicleSeat.Character)
	//		{
	//			ABaseCharacter* Character = VehicleSeat.Character;

	//			Character->SetVehicleSeat(VehicleSeat);
	//			// Attach to vehicle socket after setting vehicle seat to character as the capsule component will need to ignore the vehicle collision
	//			Character->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleSeat.SeatingSocketName);

	//			// Set to use weapon, usually a mounted gun would be used
	//			if (VehicleSeat.AssociatedWeapon > -1)
	//			{
	//				auto Index = VehicleSeat.AssociatedWeapon;
	//				auto Weapon = VehicleWeaponsPtr[Index];

	//				if (Weapon)
	//				{
	//					ACombatCharacter* CombatCharacter = Cast<ACombatCharacter>(Character);
	//					CombatCharacter->SetMountedGun(Weapon);
	//					CombatCharacter->UseMountedGun();

	//					if (VehicleWeapons[Index].AttachCharacterToWeapon)
	//					{
	//						CombatCharacter->AttachToActor(Weapon, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapons[Index].WeaponAttachmentName);
	//					}

	//					// Ignore this vehicle when using target finder component
	//					UTargetFinderComponent::AddIgnoreActor(CombatCharacter, this);
	//				}

	//			}

	//			VehicleCharactersPtr.Add(Character);

	//			//OccupiedSeats.Add(VehicleSeat);

	//		}
	//	}
	//}
}

void ALandVehicle::SpawnVehicleWeapons()
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


			Weapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, VehicleWeapon.WeaponSocketName);
			VehicleWeaponsPtr.Add(Weapon);
		}
	}

	// We need to spawn the weapons before any characters in case any associated weapons have assigned for certain seats like turrets
	SpawnVehicleSeatings();
}

void ALandVehicle::ApplyExplosionDamage(FVector ImpactPoint, FHealthParameters InHealthParams)
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
				UHealthComponent* HealthComponent = Cast<UHealthComponent>(DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

				if (HealthComponent && HealthComponent->IsAlive())
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
					HealthComponent->OnDamage(HealthParameters);
				}
			}
		}
	}
}

//void ALandVehicle::AddKill(UHealthComponent* DamagedActorHealth, UHealthComponent* OwnerHealth)
//{
//	// confirm kill if
//	// damaged actor is not the owner
//	// damaged actor is dead &
//	// damaged actor is not on the same faction side as the owner of this bullet &
//	// damaged is not neutral
//	
//
//	if (!DamagedActorHealth->IsAlive()
//		&& DamagedActorHealth->GetSelectedFaction() != OwnerHealth->GetSelectedFaction()
//		&& DamagedActorHealth->GetSelectedFaction() != TeamFaction::Neutral)
//	{
//		KillCount++;
//	}
//}

