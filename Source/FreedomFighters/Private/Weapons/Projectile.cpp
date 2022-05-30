#include "Weapons/Projectile.h"
#include "ObjectPoolActor.h"
#include "Weapons/Weapon.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "FreedomFighters/FreedomFighters.h"
#include "Characters/CombatCharacter.h"

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

AProjectile::AProjectile()
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
	Gravity = FVector(0.f, 0.f, -100.f);

	RowName = "Bullet";

	UseCustomProjectileMovement = true;
	HomingFollowWeaponEyePoint = false;
	DestroyOnDeactivate = false;
}

void AProjectile::Init()
{
	if (GetOwner())
	{
		OwningCombatCharacter = Cast<ACombatCharacter>(GetOwner());

		// The owner may not always be a character, can be a vehicle with the following components e.g. tank
		auto HealthComp = GetOwner()->GetComponentByClass(UHealthComponent::StaticClass());
		if (HealthComp)
		{
			OwnerHealth = Cast<UHealthComponent>(HealthComp);
		}

		auto FactionComp = GetOwner()->GetComponentByClass(UTeamFactionComponent::StaticClass());
		if (FactionComp)
		{
			OwnerFaction = Cast<UTeamFactionComponent>(FactionComp);
		}
	}
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// if not being used as an object pool, then perform as standard actor
	Init();

	BulletMovementAudio->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	BulletMovementAudio->SetRelativeLocation(FVector::ZeroVector);

	RetrieveSurfaceImpactSet();
}

void AProjectile::Tick(float DeltaTime)
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

void AProjectile::PlayCollisionSound(FVector Position)
{
	if (CollisionSound == nullptr) {
		return;
	}

	if (CollisionAudioComponent == nullptr) {
		CollisionAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CollisionSound, Position, FRotator::ZeroRotator, 1.f, 1.f, 0.f, ImpactAttenuation);
	}
	else {
		CollisionAudioComponent->Play();
	}
}

void AProjectile::Movement()
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

void AProjectile::FollowEyePoint()
{
	//if (WeaponParent == nullptr) {
	//	return;
	//}

	//auto EyeLocation = WeaponParent->GetEyeViewPointComponent()->GetComponentLocation();
	//auto EyeRotation = WeaponParent->GetEyeViewPointComponent()->GetComponentRotation();

	//float TraceLength = 10000.0f;
	//FVector TraceEnd = EyeLocation + (UKismetMathLibrary::GetForwardVector(EyeRotation) * TraceLength);

	//FVector TargetLocation = UKismetMathLibrary::VLerp(GetActorLocation(), TraceEnd, CurrentDeltaTime * HomingFollowFactor);
	//auto target = TargetLocation;

	//SetActorLocation(target);

	//DrawDebugLine(GetWorld(), EyeLocation, target, FColor::Green, false, 1, 0, 1);

}

void AProjectile::Activate()
{
	Super::Activate();

	KillCount = 0;

	if (TravelSound != NULL)
	{
		BulletMovementAudio->Sound = TravelSound;
		BulletMovementAudio->Play();
	}

	// The owner of this bullet can change if you swap weapons
	Init();
}

void AProjectile::Deactivate()
{
	if (DestroyOnDeactivate)
	{
		Destroy();
		return;
	}

	KillCount = 0;

	Super::Deactivate();
}

void AProjectile::RetrieveSurfaceImpactSet()
{
	if (SurfaceImpactDatatable == nullptr) {
		return;
	}

	static const FString ContextString(TEXT("Surface Impact DataSet"));
	SurfaceImpact = SurfaceImpactDatatable->FindRow<FSurfaceImpact>(RowName, ContextString, true);
}

void AProjectile::DetectHit()
{
	FHitResult OutHit;

	// Use line trace by channelto allow trace to hit on surfaces such as water where characters can move through
	// charactermesh collision profile needs to have visibility on block
	bool SphereTrace = GetWorld()->SweepSingleByChannel(
		OutHit,
		PreviousPosition,
		NextPosition,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(CapsuleComponent->GetScaledSphereRadius()),
		GetQueryParams());


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
				FHealthParameters HealthParameters;
				HealthParameters.DamagedActor = OtherActor;
				HealthParameters.DamageCauser = MyOwner;
				HealthParameters.InstigatedBy = MyOwner->GetInstigatorController();
				HealthParameters.WeaponCauser = WeaponParent;
				HealthParameters.Projectile = this;
				HealthParameters.HitInfo = OutHit;
				HealthParameters.Damage = ActualDamage;
				HealthComponent->OnDamage(HealthParameters);

				auto FactionComp = Cast<UTeamFactionComponent>(OtherActor->GetComponentByClass(UTeamFactionComponent::StaticClass()));
				AddKill(HealthComponent, FactionComp);
			}
		}
	}

	PlayCollisionSound(OutHit.ImpactPoint);

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

	FProjectileImpactParameters ProjectileImpactParameters;

	if (KillCount > 0)
	{
		if (KillCount == 1)
		{
			ProjectileImpactParameters.IsSingleKill = true;
		}
		else if (KillCount == 2)
		{
			ProjectileImpactParameters.IsDoubleKill = true;
		}
		else if (KillCount > 2)
		{
			ProjectileImpactParameters.IsMultiKill = true;
		}

		ProjectileImpactParameters.KillCount = KillCount;
		ProjectileImpactParameters.SetProjectileActor(this);

		if (OwningCombatCharacter)
		{
			OwningCombatCharacter->SetKillCount(KillCount);
		}
	}

	OnProjectileImpact.Broadcast(ProjectileImpactParameters);
	Deactivate();
}

void AProjectile::Explode(FVector ImpactPoint)
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
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, ImpactPoint, ImpactPoint, FQuat::Identity, ECC_Visibility, MyColSphere, GetQueryParams());

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

			if (!DamagedActor) {
				continue;
			}
			UHealthComponent* HealthComponent = Cast<UHealthComponent>(DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

			if (HealthComponent && HealthComponent->IsAlive())
			{
				FHealthParameters HealthParameters;
				HealthParameters.DamagedActor = DamagedActor;
				HealthParameters.DamageCauser = MyOwner;
				HealthParameters.InstigatedBy = MyOwner->GetInstigatorController();
				HealthParameters.WeaponCauser = WeaponParent;
				HealthParameters.Projectile = this;
				HealthParameters.HitInfo = Hit;
				HealthParameters.Damage = DamageAmount;
				HealthParameters.IsExplosive = isAnExplosive;
				HealthComponent->OnDamage(HealthParameters);

				auto FactionComp = Cast<UTeamFactionComponent>(DamagedActor->GetComponentByClass(UTeamFactionComponent::StaticClass()));
				AddKill(HealthComponent, FactionComp);
			}

			auto MeshComp = Cast<UStaticMeshComponent>(DamagedActor->GetRootComponent());

			if (MeshComp)
			{
				// alternivly you can use  ERadialImpulseFalloff::RIF_Linear for the impulse to get linearly weaker as it gets further from origin.
				// set the float radius to 500 and the float strength to 2000.
				MeshComp->AddRadialImpulse(ImpactPoint, ExplosiveRadius, 2000.f, ERadialImpulseFalloff::RIF_Constant, true);
			}
		}
	}
}



FSurfaceImpactSet AProjectile::CheckSurface(EPhysicalSurface SurfaceType)
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

void AProjectile::AddKill(UHealthComponent* DamagedActorHealth, UTeamFactionComponent* DamagedActorFaction)
{
	// confirm kill if
	// damaged actor is not the owner
	// damaged actor is dead &
	// damaged actor is not on the same faction side as the owner of this bullet &
	// damaged is not neutral

	if (!OwnerHealth || !OwnerFaction || !DamagedActorHealth || !DamagedActorFaction) {
		return;
	}


	if (!DamagedActorHealth->IsAlive()
		&& DamagedActorHealth != OwnerHealth
		&& OwnerFaction->GetSelectedFaction() != DamagedActorFaction->GetSelectedFaction()
		&& DamagedActorFaction->GetSelectedFaction() != TeamFaction::Neutral)
	{
		KillCount++;
	}
}

FCollisionQueryParams AProjectile::GetQueryParams()
{
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(this);

	if (WeaponParent) {
		QueryParams.AddIgnoredActor(WeaponParent);
	}

	// weapon bullets can often collide with owning character, 
	// but explosion may set ignore owner to false
	if (IgnoreOwner)
	{
		if (GetOwner()) {
			QueryParams.AddIgnoredActor(GetOwner());
		}
	}

	return QueryParams;
}