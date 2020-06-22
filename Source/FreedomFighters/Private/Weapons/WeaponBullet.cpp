// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBullet.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"

#include <array>

AWeaponBullet::AWeaponBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetCollisionProfileName(TEXT("WeaponProjectile"));
	Root->CanCharacterStepUpOn = ECB_No;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->AttachTo(Root);
	Mesh->SetCollisionProfileName(TEXT("WeaponProjectile"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("BulletMovement"));
	BulletMovement->InitialSpeed = 2000.0f;
	BulletMovement->MaxSpeed = 2000.0f;


	DamageAmount = 5.0f;
	DamageRadius = 10.0f;


}

void AWeaponBullet::Explode()
{
	UWorld* world = GetWorld();

	if (world)
	{
		//DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 20, FColor::Purple, false, -1, 0, 1);

		if (ImpactParticle != NULL)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorLocation());
		}

		if (ImpactSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(world, ImpactSound, GetActorLocation(), 1.0f);
		}

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
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		Explode();
	}
}

void AWeaponBullet::BeginPlay()
{
	Super::BeginPlay();

	Root->OnComponentHit.AddDynamic(this, &AWeaponBullet::OnBulletHit);

}

void AWeaponBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



