// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBullet.h"

#include "FreedomFighters/FreedomFighters.h"
#include "CustomComponents/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
//#include "DestructibleComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"

#include <array>

AWeaponBullet::AWeaponBullet()
{
	PrimaryActorTick.bCanEverTick = false;

	CapsuleComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetNotifyRigidBodyCollision(true);
	CapsuleComponent->SetCollisionProfileName(TEXT("WeaponProjectile"));
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->SetCollisionProfileName(TEXT("WeaponProjectile"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("BulletMovement"));
	BulletMovement->InitialSpeed = 12000.0;
	BulletMovement->MaxSpeed = 15000.0;

	DamageAmount = 15.0f;
	DamageRadius = 10.0f;
}

void AWeaponBullet::BeginPlay()
{
	Super::BeginPlay();

	CapsuleComponent->OnComponentHit.AddDynamic(this, &AWeaponBullet::OnBulletHit);
	Mesh->OnComponentHit.AddDynamic(this, &AWeaponBullet::OnBulletHit);
}


void AWeaponBullet::Explode()
{
	UWorld* world = GetWorld();

	if (world)
	{
		//DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 20, FColor::Purple, false, -1, 0, 1);

		TArray<AActor*> ignoredActors;
		UGameplayStatics::ApplyRadialDamage(world, DamageAmount, GetActorLocation(), DamageRadius, NULL, ignoredActors, this, AActor::GetInstigatorController());

		// create tarray for hit results
		TArray<FHitResult> OutHits;

		// get actor locations
		FVector MyLocation = GetActorLocation();

		// create a collision sphere
		FCollisionShape MyColSphere = FCollisionShape::MakeSphere(DamageRadius);

		// draw collision sphere
		//DrawDebugSphere(GetWorld(), GetActorLocation(), MyColSphere.GetSphereRadius(), 50, FColor::Cyan, true);

		// check if something got hit in the sweep
		bool isHit = GetWorld()->SweepMultiByChannel(OutHits, MyLocation, MyLocation, FQuat::Identity, ECC_WorldStatic, MyColSphere);

		if (isHit)
		{
			// loop through TArray
			for (auto& Hit : OutHits)
			{
				UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>((Hit.GetActor())->GetRootComponent());

				if (MeshComp)
				{
					// alternivly you can use  ERadialImpulseFalloff::RIF_Linear for the impulse to get linearly weaker as it gets further from origin.
					// set the float radius to 500 and the float strength to 2000.
					MeshComp->AddRadialImpulse(GetActorLocation(), DamageRadius, 2000.f, ERadialImpulseFalloff::RIF_Constant, true);
				}
			}
		}

		Destroy();
	}


}

void AWeaponBullet::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != NULL && OtherActor != this)
	{

		float ActualDamage = DamageAmount;
		EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
		if (SurfaceType == SURFACE_HEAD)
		{
			ActualDamage = 100;
		}
		else if (SurfaceType == SURFACE_FLESHVULNERABLE)
		{
			ActualDamage *= 2.0f;
		}



		UHealthComponent* HealthComponent = Cast<UHealthComponent>(OtherActor->GetComponentByClass(UHealthComponent::StaticClass()));

		if (HealthComponent)
		{
			HealthComponent->SetHitInfo(Hit);
		}

		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			UGameplayStatics::ApplyPointDamage(OtherActor, ActualDamage, FVector::ZeroVector, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
		}

		SetDestructableHit(OtherActor);

		if (ExplosionParticle != NULL)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, Hit.ImpactPoint);
		}

		if (ImpactSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Hit.ImpactPoint, 1.0f);
		}

		if (isAnExplosive) {
			Explode();
		}
		else
		{
			Destroy();
		}

		UParticleSystem* ImpactParticle = CheckSurface(SurfaceType);
		if (ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}

	}
}

UParticleSystem* AWeaponBullet::CheckSurface(EPhysicalSurface SurfaceType)
{
	switch (SurfaceType)
	{
	case SURFACE_HEAD:
	case SURFACE_GROIN:
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		return FleshImpactEffect;
		break;
	default:
		return DefaultImpactEffect;
		break;
	}
}


void AWeaponBullet::SetDestructableHit(AActor* OtherActor)
{
	//UDestructibleComponent* DestructibleComponent = Cast<UDestructibleComponent>(OtherActor->GetComponentByClass(UDestructibleComponent::StaticClass()));

	//if (DestructibleComponent) {
	//	DestructibleComponent->SetSimulatePhysics(true);
	//}
}