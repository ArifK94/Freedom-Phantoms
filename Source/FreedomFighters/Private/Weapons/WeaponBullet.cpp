#include "Weapons/WeaponBullet.h"

#include "FreedomFighters/FreedomFighters.h"
#include "CustomComponents/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "DrawDebugHelpers.h"

AWeaponBullet::AWeaponBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetCollisionProfileName(TEXT("NoCollision"));
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	Mesh->CanCharacterStepUpOn = ECB_No;

	BulletMovementAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BulletMovementAudio"));

	DamageAmount = 15.0f;
	ExplosiveRadius = 10.0f;

	ShowExplosionRadius = false;
	DebugExplosionLifeTime = 5.0f;

	InitialSpeed = 12000.0;
	Mass = 500.f;
	Drag = 0.1f;
	Gravity = FVector(0.f, 0.f, -980.f);
}

void AWeaponBullet::BeginPlay()
{
	Super::BeginPlay();

	BulletMovementAudio->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	BulletMovementAudio->SetRelativeLocation(FVector::ZeroVector);
}

void AWeaponBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	//if (!IsActive()) {
	//	return;
	//}

	Movement();
}

void AWeaponBullet::Movement()
{
	// Get projectile's location at beginning of tick
	PreviousPosition = GetActorLocation();

	Velocity = GetActorForwardVector() * InitialSpeed;

	// Calculate Drag
	FVector DragStrength = (Velocity * Velocity.Size()) * (Drag / Mass);

	// Calculate acceleration and total position offset
	Acceleration = Gravity - DragStrength;

	// Calculate position offset
	NextPosition = Acceleration * UKismetMathLibrary::MultiplyMultiply_FloatFloat(CurrentDeltaTime, 2.0f) * 0.5f + Velocity * CurrentDeltaTime + GetActorLocation();

	// Calculate velocity for next tick
	Velocity = Velocity + Acceleration * CurrentDeltaTime;

	// Set final position & rotation
	SetActorLocation(NextPosition);
	SetActorRotation(UKismetMathLibrary::MakeRotFromX(Velocity));

	DetectHit();
}

void AWeaponBullet::Activate()
{
	if (TravelSound != NULL)
	{
		BulletMovementAudio->Sound = TravelSound;
		BulletMovementAudio->Play();
	}

	Super::Activate();
}

void AWeaponBullet::DetectHit()
{
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(this);

	// weapon bullets can often collide with owning character, 
	// but explosion may set ignore owner to false
	if (IgnoreOwner)
	{
		if (GetOwner()) {
			QueryParams.AddIgnoredActor(GetOwner());
		}
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;


	FHitResult OutHit;
	bool SphereTrace = GetWorld()->SweepSingleByObjectType(
		OutHit,
		PreviousPosition,
		NextPosition,
		FQuat(),
		ObjectParams,
		FCollisionShape::MakeSphere(CapsuleComponent->GetScaledSphereRadius()),
		QueryParams
	);


	if (SphereTrace)
	{
		if (OutHit.bBlockingHit)
		{
			AActor* OtherActor = OutHit.GetActor();

			float ActualDamage = DamageAmount;
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(OutHit.PhysMaterial.Get());
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

			if (isAnExplosive) {
				Explode(OutHit.ImpactPoint);
			}
			else
			{

				AActor* MyOwner = GetOwner();
				if (MyOwner)
				{
					UHealthComponent* HealthComponent = Cast<UHealthComponent>(OtherActor->GetComponentByClass(UHealthComponent::StaticClass()));

					if (HealthComponent)
					{
						HealthComponent->OnDamage(OtherActor, ActualDamage, NULL, MyOwner->GetInstigatorController(), MyOwner, WeaponParent, this, OutHit);
					}
				}

				Deactivate();
			}

			if (ImpactSound != NULL)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, OutHit.ImpactPoint, 1.0f, 1.0f, 0.0f, ImpactAttenuation);
			}

			UParticleSystem* ImpactParticle = CheckSurface(SurfaceType);
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}

		}
	}

}

void AWeaponBullet::Explode(FVector ImpactPoint)
{
	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	AActor* MyOwner = GetOwner();

	if (ExplosionParticle != NULL)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorLocation());
	}

	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(ExplosiveRadius);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, ImpactPoint, ImpactPoint, FQuat::Identity, ECC_Visibility, MyColSphere);

	if (ShowExplosionRadius)
	{
		DrawDebugSphere(GetWorld(), ImpactPoint, ExplosiveRadius, 20, FColor::Purple, false, DebugExplosionLifeTime, 0, 2);
	}

	if (isHit)
	{
		// loop through TArray
		for (auto& Hit : OutHits)
		{
			AActor* DamagedActor = Hit.GetActor();
			if (DamagedActor)
			{
				UHealthComponent* HealthComponent = Cast<UHealthComponent>(DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

				if (HealthComponent)
				{
					HealthComponent->OnDamage(DamagedActor, DamageAmount, NULL, MyOwner->GetInstigatorController(), MyOwner, WeaponParent, this, Hit);
				}
			}

			UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>((Hit.GetActor())->GetRootComponent());

			if (MeshComp)
			{
				// alternivly you can use  ERadialImpulseFalloff::RIF_Linear for the impulse to get linearly weaker as it gets further from origin.
				// set the float radius to 500 and the float strength to 2000.
				MeshComp->AddRadialImpulse(ImpactPoint, ExplosiveRadius, 2000.f, ERadialImpulseFalloff::RIF_Constant, true);
			}
		}
	}

	Deactivate();
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