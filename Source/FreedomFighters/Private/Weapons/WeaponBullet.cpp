#include "Weapons/WeaponBullet.h"

#include "FreedomFighters/FreedomFighters.h"
#include "CustomComponents/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"

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

	BulletMovementAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BulletMovementAudio"));

	DamageAmount = 15.0f;
	DamageRadius = 10.0f;

	ShowExplosionRadius = false;
	DebugExplosionLifeTime = 5.0f;
}

void AWeaponBullet::BeginPlay()
{
	Super::BeginPlay();

	BulletMovement->Deactivate();

	CapsuleComponent->OnComponentHit.AddDynamic(this, &AWeaponBullet::OnBulletHit);
	Mesh->OnComponentHit.AddDynamic(this, &AWeaponBullet::OnBulletHit);

	BulletMovementAudio->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	BulletMovementAudio->SetRelativeLocation(FVector::ZeroVector);

}

void AWeaponBullet::Activate()
{
	UProjectileMovementComponent* ProjectileMovementComp = Cast<UProjectileMovementComponent>(GetComponentByClass(UProjectileMovementComponent::StaticClass()));

	if (ProjectileMovementComp)
	{
		UProjectileMovementComponent* NewProjectileMovementComp = NewObject<UProjectileMovementComponent>(this);
		NewProjectileMovementComp->InitialSpeed = ProjectileMovementComp->InitialSpeed;
		NewProjectileMovementComp->MaxSpeed = ProjectileMovementComp->MaxSpeed;
		NewProjectileMovementComp->Bounciness = ProjectileMovementComp->Bounciness;
		NewProjectileMovementComp->Friction = ProjectileMovementComp->Friction;
		NewProjectileMovementComp->ProjectileGravityScale = ProjectileMovementComp->ProjectileGravityScale;
		// delete it before registering new Projectile Movement Component in case any physics are applied and cause a clash between two of the componentss
		ProjectileMovementComp->DestroyComponent();
		NewProjectileMovementComp->RegisterComponent();
	}
	else
	{
		ProjectileMovementComp->Deactivate();
	}

	if (TravelSound != NULL)
	{
		BulletMovementAudio->Sound = TravelSound;
		BulletMovementAudio->Play();
	}

	Super::Activate();
}

void AWeaponBullet::Explode(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		TArray<AActor*> ignoredActors;

		UGameplayStatics::ApplyRadialDamage(
			World,
			DamageAmount,
			GetActorLocation(),
			DamageRadius,
			DamageType,
			ignoredActors,
			this, // damage does not work if assiginng the owner, therefore health component will need to get the owner of this bullet class
			MyOwner->GetInstigatorController()
		);

		if (ShowExplosionRadius)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), DamageRadius, 20, FColor::Purple, false, DebugExplosionLifeTime, 0, 2);
		}
	}

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

	Deactivate();
}

void AWeaponBullet::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == NULL || OtherActor == this) {
		return;
	}

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

	if (BulletMovementAudio->IsPlaying())
	{
		BulletMovementAudio->Stop();
	}

	UHealthComponent* HealthComponent = Cast<UHealthComponent>(OtherActor->GetComponentByClass(UHealthComponent::StaticClass()));

	if (HealthComponent)
	{
		HealthComponent->SetHitInfo(Hit);
	}

	if (isAnExplosive) {
		Explode(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	}
	else
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			UGameplayStatics::ApplyPointDamage(OtherActor, ActualDamage, FVector::ZeroVector, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);
		}

		Deactivate();
	}

	if (ExplosionParticle != NULL)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, Hit.ImpactPoint);
	}

	if (ImpactSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Hit.ImpactPoint, 1.0f);
	}

	UParticleSystem* ImpactParticle = CheckSurface(SurfaceType);
	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
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