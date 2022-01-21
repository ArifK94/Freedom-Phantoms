#include "Weapons/WeaponBullet.h"
#include "ObjectPoolActor.h"
#include "Weapons/Weapon.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "Characters/CombatCharacter.h"
#include "FreedomFighters/FreedomFighters.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"

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

	RowName = "Bullet";

	UseCustomProjectileMovement = true;
	HomingFollowWeaponEyePoint = false;
	DestroyOnDeactivate = false;
}

void AWeaponBullet::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		OwningCombatCharacter = Cast<ACombatCharacter>(GetOwner());
		OwnerHealth = OwningCombatCharacter->GetHealthComp();
	}

	BulletMovementAudio->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	BulletMovementAudio->SetRelativeLocation(FVector::ZeroVector);

	RetrieveSurfaceImpactSet();
}

void AWeaponBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentDeltaTime = DeltaTime;

	if (UseCustomProjectileMovement)
	{
		Movement();

		//if (HomingFollowWeaponEyePoint)
		//{
		//	if (DestroyOnDeactivate) // if not using this projectile in the object pool
		//	{
		//		FollowEyePoint();
		//	}
		//	else // if it is object pooled
		//	{
		//		if (IsActive()) // if activated
		//		{
		//			FollowEyePoint();
		//		}
		//	}
		//}
	}


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

void AWeaponBullet::FollowEyePoint()
{
	if (WeaponParent == nullptr) {
		return;
	}

	auto EyeLocation = WeaponParent->GetEyeViewPointComponent()->GetComponentLocation();
	auto EyeRotation = WeaponParent->GetEyeViewPointComponent()->GetComponentRotation();

	float TraceLength = 10000.0f;
	FVector TraceEnd = EyeLocation + (UKismetMathLibrary::GetForwardVector(EyeRotation) * TraceLength);

	FVector TargetLocation = UKismetMathLibrary::VLerp(GetActorLocation(), TraceEnd, CurrentDeltaTime * HomingFollowFactor);
	auto target = TargetLocation;

	//SetActorLocation(target);

	//DrawDebugLine(GetWorld(), EyeLocation, target, FColor::Green, false, 1, 0, 1);

}

void AWeaponBullet::Activate()
{
	Super::Activate();

	KillCount = 0;

	if (TravelSound != NULL)
	{
		BulletMovementAudio->Sound = TravelSound;
		BulletMovementAudio->Play();
	}

	if (GetOwner())
	{
		OwningCombatCharacter = Cast<ACombatCharacter>(GetOwner());
		OwnerHealth = OwningCombatCharacter->GetHealthComp();
		OwnerFaction = OwningCombatCharacter->GetTeamFactionComponent();
	}
}

void AWeaponBullet::Deactivate()
{
	if (DestroyOnDeactivate)
	{
		Destroy();
		return;
	}

	KillCount = 0;

	Super::Deactivate();
}

void AWeaponBullet::RetrieveSurfaceImpactSet()
{
	if (SurfaceImpactDatatable == nullptr) {
		return;
	}

	static const FString ContextString(TEXT("Surface Impact DataSet"));
	SurfaceImpact = SurfaceImpactDatatable->FindRow<FSurfaceImpact>(RowName, ContextString, true);
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
	//bool SphereTrace = GetWorld()->SweepSingleByObjectType(
	//	OutHit,
	//	PreviousPosition,
	//	NextPosition,
	//	FQuat(),
	//	ObjectParams,
	//	FCollisionShape::MakeSphere(CapsuleComponent->GetScaledSphereRadius()),
	//	QueryParams
	//);

	// Use line trace by channelto allow trace to hit on surfaces such as water where characters can move through
	// charactermesh collision profile needs to have visibility on block
	bool SphereTrace = GetWorld()->SweepSingleByChannel(
		OutHit, 
		PreviousPosition, 
		NextPosition, 
		FQuat::Identity, 
		ECC_Visibility,
		FCollisionShape::MakeSphere(CapsuleComponent->GetScaledSphereRadius()),
		QueryParams);


	if (!SphereTrace)
	{
		return;
	}
	if (!OutHit.bBlockingHit)
	{
		return;
	}

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

			if (HealthComponent && HealthComponent->IsAlive())
			{
				HealthComponent->OnDamage(OtherActor, ActualDamage, NULL, MyOwner->GetInstigatorController(), MyOwner, WeaponParent, this, OutHit);

				auto FactionComp = Cast<UTeamFactionComponent>(OtherActor->GetComponentByClass(UTeamFactionComponent::StaticClass()));
				AddKill(HealthComponent, FactionComp);
			}
		}
	}

	FSurfaceImpactSet ImpactSurface = CheckSurface(SurfaceType);

	if (ImpactSurface.Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSurface.Sound, OutHit.ImpactPoint, 1.0f, 1.0f, 0.0f, ImpactAttenuation);
	}

	if (ImpactSurface.ParticleEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactSurface.ParticleEffect, OutHit.ImpactPoint);
	}

	if (ImpactSurface.NiagaraEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactSurface.NiagaraEffect, OutHit.ImpactPoint);
	}


	if (KillCount > 0)
	{
		bool IsSingleKill = false;
		bool IsDoubleKill = false;
		bool IsMultiKill = false;

		if (KillCount == 1)
		{
			IsSingleKill = true;
		}
		else if (KillCount == 2)
		{
			IsDoubleKill = true;
		}
		else if (KillCount > 2)
		{
			IsMultiKill = true;
		}

		OwningCombatCharacter->SetKillCount(KillCount);

		OnKillConfirmed.Broadcast(KillCount, IsSingleKill, IsDoubleKill, IsMultiKill);
	}

	Deactivate();
}

void AWeaponBullet::Explode(FVector ImpactPoint)
{
	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	AActor* MyOwner = GetOwner();

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

				if (HealthComponent && HealthComponent->IsAlive())
				{
					HealthComponent->OnDamage(DamagedActor, DamageAmount, NULL, MyOwner->GetInstigatorController(), MyOwner, WeaponParent, this, Hit);

					auto FactionComp = Cast<UTeamFactionComponent>(DamagedActor->GetComponentByClass(UTeamFactionComponent::StaticClass()));
					AddKill(HealthComponent, FactionComp);
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
}



FSurfaceImpactSet AWeaponBullet::CheckSurface(EPhysicalSurface SurfaceType)
{
	FSurfaceImpactSet SurfaceImpactSet = SurfaceImpact->Default;

	switch (SurfaceType)
	{
	case SURFACE_HEAD:
	case SURFACE_GROIN:
	case SURFACE_LEGS:
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SurfaceImpactSet = SurfaceImpact->Flesh;
		break;
	case SURFACE_WATER:
		SurfaceImpactSet = SurfaceImpact->Water;
		break;
	case SURFACE_GRASS:
		SurfaceImpactSet = SurfaceImpact->Grass;
		break;
	case SURFACE_WOOD:
		SurfaceImpactSet = SurfaceImpact->Wood;
		break;
	case SURFACE_ROCK:
		SurfaceImpactSet = SurfaceImpact->Rock;
		break;
	case SURFACE_SAND:
		SurfaceImpactSet = SurfaceImpact->Sand;
		break;
	}

	return SurfaceImpactSet;
}

void AWeaponBullet::AddKill(UHealthComponent* DamagedActorHealth, UTeamFactionComponent* DamagedActorFaction)
{
	// confirm kill if
	// damaged actor is not the owner
	// damaged actor is dead &
	// damaged actor is not on the same faction side as the owner of this bullet &
	// damaged is not neutral

	if (!DamagedActorHealth || !OwnerFaction || !DamagedActorFaction) {
		return;
	}


	if (!DamagedActorHealth->IsAlive()
		&& DamagedActorHealth != OwnerHealth
		&& OwnerFaction->GetSelectedFaction() != DamagedActorFaction->GetSelectedFaction()
		&& DamagedActorFaction->GetSelectedFaction()!= TeamFaction::Neutral)
	{
		KillCount++;
	}
}