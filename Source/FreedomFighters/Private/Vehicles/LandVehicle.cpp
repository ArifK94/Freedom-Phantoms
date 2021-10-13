// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/LandVehicle.h"
#include "CustomComponents/HealthComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"

ALandVehicle::ALandVehicle()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ALandVehicle::OnHealthChanged);

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250.0f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true; // ignore self

	ExplosionImpulse = 400.0f;
	ExplosionDamage = 30.0f;

	SimulateExplosionPhysics = false;
}

void ALandVehicle::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALandVehicle::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	if (IsDestroyed) {
		return;
	}

	if (Health <= 0.0f)
	{
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
		ApplyExplosionDamage(GetActorLocation(), InstigatedBy, DamageCauser, WeaponCauser, Bullet);
	}
}

void ALandVehicle::ApplyExplosionDamage(FVector ImpactPoint, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet)
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
					HealthComponent->OnDamage(DamagedActor, ExplosionDamage, NULL, InstigatedBy, DamageCauser, WeaponCauser, Bullet, Hit);

					AddKill(HealthComponent);
				}
			}
		}
	}
}

void ALandVehicle::AddKill(UHealthComponent* DamagedActorHealth)
{
	// confirm kill if
	// damaged actor is not the owner
	// damaged actor is dead &
	// damaged actor is not on the same faction side as the owner of this bullet &
	// damaged is not neutral
	

	//if (!DamagedActorHealth->IsAlive()
	//	&& DamagedActorHealth->GetSelectedFaction() != OwnerHealth->GetSelectedFaction()
	//	&& DamagedActorHealth->GetSelectedFaction() != TeamFaction::Neutral)
	//{
	//	KillCount++;
	//}
}

