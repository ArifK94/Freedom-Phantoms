#include "Weapons/Projectile.h"
#include "ObjectPoolActor.h"
#include "Weapons/Weapon.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "FreedomPhantoms/FreedomPhantoms.h"
#include "Characters/CombatCharacter.h"
#include "Interfaces/Avoidable.h"
#include "Interfaces/Targetable.h"
#include "StructCollection.h"
#include "Services/SharedService.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
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
	ExplosiveRadiusInner = 10.0f;
	ExplosiveRadiusOuter = 20.f;
	RadialForceStrength = 200.f;

	ShowDebug = false;
	DebugExplosionLifeTime = 5.0f;

	InitialSpeed = 12000.0;
	Mass = 500.f;
	Drag = 0.1f;
	Gravity = FVector(0.f, 0.f, -100.f);
	CountdownTimer = -1.f;

	RowName = "Bullet";

	UseCustomProjectileMovement = true;
	UseLaserGuidance = false;
	DestroyOnDeactivate = false;
	DetectProjectileHit = true;
	DetectNearbyActors = false;
	SpinOnVelocity = false;

	DecalSizeMin = 150.f;
	DecalSizeMax = 300.f;
	DecalRotationMin = -360.f;
	DecalRotationMax = 360.f;
	DecalLifetime = 5.f;
	DecalFadeOutDuration = 10.f;

	CamShakeOuterRadius = 2000.f;
}

void AProjectile::Init()
{
	if (CountdownTimer > 0.f) {
		GetWorldTimerManager().SetTimer(THandler_CountdownTimer, this, &AProjectile::SelfDestruct, 1.f, true, CountdownTimer);
	}

	if (GetOwner()) {
		OwningCombatCharacter = Cast<ACombatCharacter>(GetOwner());

		// The owner may not always be a character, can be a vehicle with the following components e.g. tank
		auto HealthComp = GetOwner()->GetComponentByClass(UHealthComponent::StaticClass());
		if (HealthComp) {
			OwnerHealth = Cast<UHealthComponent>(HealthComp);
		}

		auto FactionComp = GetOwner()->GetComponentByClass(UTeamFactionComponent::StaticClass());
		if (FactionComp) {
			OwnerFaction = Cast<UTeamFactionComponent>(FactionComp);
		}
	}

	if (!DetectionSphere) {
		DetectionSphere = NewObject<USphereComponent>(this);

		if (DetectionSphere) {
			DetectionSphere->RegisterComponent();
			DetectionSphere->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			DetectionSphere->SetSphereRadius(ExplosiveRadiusOuter);
			DetectionSphere->SetCanEverAffectNavigation(false);
			DetectionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
		}
	}
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	CapsuleComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnCapsuleHit);

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

	if (UseCustomProjectileMovement) {
		Movement();
	}

	if (SpinOnVelocity) {
		SpinOnMovement();
	}

	AlertNearbyActors();
}

void AProjectile::OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	PlayCollisionSound(NormalImpulse);

	LastHit = Hit;
}

void AProjectile::OnImpactHit(FHitResult InHit)
{
	AActor* OtherActor = InHit.GetActor();

	float ActualDamage = DamageAmount;
	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(InHit.PhysMaterial.Get());

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
		Explode(InHit.ImpactPoint);
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
				HealthParameters.HitInfo = InHit;
				HealthParameters.Damage = ActualDamage;
				HealthComponent->OnDamage(HealthParameters);

				auto FactionComp = Cast<UTeamFactionComponent>(OtherActor->GetComponentByClass(UTeamFactionComponent::StaticClass()));
				AddKill(HealthComponent, FactionComp);
			}
		}
	}

	PlayCollisionSound(InHit.ImpactPoint);

	FSurfaceImpactSet ImpactSurface = CheckSurface(SurfaceType);
	SetVFX(ImpactSurface, InHit.ImpactPoint);

	FProjectileImpactParameters ProjectileImpactParameters;

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
		OwningCombatCharacter->AddKillCount(KillCount);
	}

	OnProjectileImpact.Broadcast(ProjectileImpactParameters);
	Deactivate();
}

void AProjectile::PlayCollisionSound(FVector Position)
{
	if (CollisionSound == nullptr) {
		return;
	}

	if (CollisionAudioComponent == nullptr) {
		CollisionAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CollisionSound, Position, FRotator::ZeroRotator, 1.f, 1.f, 0.f, CollisionAttenuation);
	}
	else {
		CollisionAudioComponent->Play();
	}
}

void AProjectile::Movement()
{
	if (UseLaserGuidance)
	{
		LaserGuidance();
	}
	else
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
	}

	if (DetectProjectileHit) {
		DetectHit();
	}
}

void AProjectile::LaserGuidance()
{
	if (WeaponParent == nullptr) {
		return;
	}

	auto EyeLocation = WeaponParent->GetEyeViewPointComponent()->GetComponentLocation();
	auto EyeRotation = WeaponParent->GetEyeViewPointComponent()->GetComponentRotation();

	FVector EyeForward = EyeRotation.Vector();
	FVector TraceEnd = EyeLocation + (EyeForward * 1000000000.f);

	FVector Direction = TraceEnd - GetActorLocation();
	Direction.Normalize();
	FRotator DesiredRotation = Direction.Rotation();

	FRotator MyRotation = GetActorRotation();
	FRotator InterpRotation = FMath::RInterpConstantTo(MyRotation, DesiredRotation, CurrentDeltaTime, 5.f);
	
	// Get projectile's location at beginning of tick
	PreviousPosition = GetActorLocation();

	// Calculate Drag
	FVector DragStrength = (Velocity * Velocity.Size()) * (Drag / Mass);

	// Calculate acceleration and total position offset
	Acceleration = (Direction * InitialSpeed) - DragStrength;

	// Calculate position offset
	NextPosition = Acceleration * UKismetMathLibrary::MultiplyMultiply_FloatFloat(CurrentDeltaTime, 2.0f) * 0.5f + Velocity * CurrentDeltaTime + GetActorLocation();

	// Calculate velocity for next tick
	Velocity = Velocity + Acceleration * CurrentDeltaTime;



	// Set final position & rotation
	SetActorLocation(NextPosition);
	SetActorRotation(InterpRotation);
}

void AProjectile::SpinOnMovement()
{
	auto CurrentSpeed = GetVelocity().Size();

	if (CurrentSpeed <= 0.f) {
		return;
	}

	// slow down the rotation 
	//CurrentSpeed /= 10.f;

	FRotator NewRotation = CurrentRotation;
	NewRotation.Pitch += CurrentRotation.Pitch + CurrentSpeed;
	NewRotation.Yaw += CurrentRotation.Yaw + CurrentSpeed;
	NewRotation.Roll += CurrentRotation.Roll + CurrentSpeed;

	//AddActorWorldRotation(NewRotation);
	CurrentRotation = NewRotation;
}

void AProjectile::Activate()
{
	Super::Activate();

	IsDestroyed = false;

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
	if (HomingTargetActor && HomingTargetActor->GetClass()->ImplementsInterface(UTargetable::StaticClass())) {
		ITargetable::Execute_OnMissileDestroyed(HomingTargetActor, this);
	}



	if (DestroyOnDeactivate)
	{
		Destroy();
	}
	else
	{
		IsDestroyed = true;

		GetWorldTimerManager().ClearTimer(THandler_CountdownTimer);

		KillCount = 0;

		if (DetectionSphere) {
			DetectionSphere->SetCollisionProfileName(TEXT("NoCollision"));
		}

		Super::Deactivate();
	}
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

	OnImpactHit(OutHit);
}

void AProjectile::Explode(FVector ImpactPoint)
{
	if (IsDestroyed) {
		return;
	}

	// required to prevent stack overflow exception if destroying nearby explosives.
	IsDestroyed = true;

	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	AActor* MyOwner = GetOwner();


	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(ExplosiveRadiusOuter);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, ImpactPoint, ImpactPoint, FQuat::Identity, ECC_Visibility, MyColSphere, GetQueryParams());

	if (ShowDebug)
	{
		DrawDebugSphere(GetWorld(), ImpactPoint, ExplosiveRadiusInner, 20, FColor::Red, false, DebugExplosionLifeTime, 0, 2);
		DrawDebugSphere(GetWorld(), ImpactPoint, ExplosiveRadiusOuter, 20, FColor::Green, false, DebugExplosionLifeTime, 0, 2);
	}

	if (isHit)
	{
		TArray<AActor*> DamagedActors;
		TArray<FHitResult> DamagedActorsHitInfo;

		// loop through TArray
		for (auto& Hit : OutHits)
		{
			AActor* DamagedActor = Hit.GetActor();

			if (!DamagedActor) {
				continue;
			}

			if (!DamagedActors.Contains(DamagedActor)) {
				DamagedActors.Add(DamagedActor);
				DamagedActorsHitInfo.Add(Hit);
			}
		}

		for (auto i = 0; i < DamagedActors.Num(); i++)
		{
			AActor* DamagedActor = DamagedActors[i];

			UHealthComponent* HealthComponent = Cast<UHealthComponent>(DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

			if (HealthComponent && HealthComponent->IsAlive())
			{
				float Distance = UKismetMathLibrary::Vector_Distance(DamagedActor->GetActorLocation(), ImpactPoint);

				// health affected if within inner radius
				if (IsHittingTarget(DamagedActor, ImpactPoint, ExplosiveRadiusInner))
				{
					float newDamage = FMath::Clamp((DamageAmount * ExplosiveRadiusInner) / Distance, 0.f, DamageAmount);
					newDamage = FMath::Abs(newDamage);

					FHealthParameters HealthParameters;
					HealthParameters.DamagedActor = DamagedActor;
					HealthParameters.DamageCauser = MyOwner;
					HealthParameters.InstigatedBy = MyOwner ? MyOwner->GetInstigatorController() : nullptr;
					HealthParameters.WeaponCauser = WeaponParent;
					HealthParameters.Projectile = this;
					HealthParameters.HitInfo = DamagedActorsHitInfo[i];
					HealthParameters.Damage = newDamage;
					HealthParameters.IsExplosive = isAnExplosive;
					HealthComponent->OnDamage(HealthParameters);

					auto FactionComp = Cast<UTeamFactionComponent>(DamagedActor->GetComponentByClass(UTeamFactionComponent::StaticClass()));
					AddKill(HealthComponent, FactionComp);
				}

			}


			auto HitProjectile = Cast<AProjectile>(DamagedActor);
			if (HitProjectile && HitProjectile->IsExplosive() && !HitProjectile->GetIgnoreDamage()) {
				HitProjectile->SelfDestruct();
			}

			auto MeshComp = Cast<UStaticMeshComponent>(DamagedActor->GetRootComponent());

			if (MeshComp)
			{
				// alternivly you can use  ERadialImpulseFalloff::RIF_Linear for the impulse to get linearly weaker as it gets further from origin.
				// set the float radius to 500 and the float strength to 2000.
				MeshComp->AddRadialImpulse(ImpactPoint, ExplosiveRadiusInner, RadialForceStrength, ERadialImpulseFalloff::RIF_Constant, true);
			}
		}
	}
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

void AProjectile::SelfDestruct()
{
	FVector Location = GetActorLocation();
	Explode(Location);

	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(LastHit.PhysMaterial.Get());
	FSurfaceImpactSet ImpactSurface = CheckSurface(SurfaceType);
	SetVFX(ImpactSurface, Location);


	FProjectileImpactParameters ProjectileImpactParameters;

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
		OwningCombatCharacter->AddKillCount(KillCount);
	}

	OnProjectileImpact.Broadcast(ProjectileImpactParameters);

	Deactivate();
}

bool AProjectile::IsHittingTarget(AActor* TargetActor, FVector ImpactPoint, float Radius)
{
	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(Radius);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, ImpactPoint, ImpactPoint, FQuat::Identity, ECC_Visibility, MyColSphere, GetQueryParams());

	if (!isHit) {
		return false;
	}

	if (ShowDebug)
	{
		DrawDebugSphere(GetWorld(), ImpactPoint, Radius, 20, FColor::Red, false, DebugExplosionLifeTime, 0, 2);
	}

	for (auto& Hit : OutHits)
	{
		AActor* DamagedActor = Hit.GetActor();

		if (ShowDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("DamagedActor: %s | TargetActor: %s"), *DamagedActor->GetName(), *TargetActor->GetName()));
		}

		if (DamagedActor == TargetActor || (DamagedActor->GetAttachParentActor() && DamagedActor->GetAttachParentActor() == TargetActor)) {

			if (ShowDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("TargetActor Hit!")));

				if (DamagedActor->GetAttachParentActor())
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("DamagedActor: %s"), *DamagedActor->GetAttachParentActor()->GetName()));
				}

			}

			return true;
		}
	}

	return false;
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
	default:
		SurfaceImpactSet = SurfaceImpact->Default;
		break;
	}

	return SurfaceImpactSet;
}

void AProjectile::SetVFX(FSurfaceImpactSet ImpactSurface, FVector ImpactLocation)
{
	ImpactLocation = ImpactLocation + ImpactSurface.VFXOffset.GetLocation();
	FRotator ImpactRotation = ImpactSurface.VFXOffset.GetRotation().Rotator();

	if (ImpactSurface.Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSurface.Sound, ImpactLocation, 1.0f, 1.0f, 0.0f, ImpactAttenuation);
	}

	if (IsInAir())
	{
		if (ImpactSurface.AirParticleEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactSurface.AirParticleEffect, ImpactLocation, ImpactRotation);
		}

		if (ImpactSurface.AirNiagaraEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactSurface.AirNiagaraEffect, ImpactLocation, ImpactRotation);
		}
	}
	else
	{
		if (ImpactSurface.ParticleEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactSurface.ParticleEffect, ImpactLocation, ImpactRotation);
		}

		if (ImpactSurface.NiagaraEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactSurface.NiagaraEffect, ImpactLocation, ImpactRotation);
		}
	}

	if (ImpactSurface.DecalMaterial)
	{
		float Size = UKismetMathLibrary::RandomFloatInRange(DecalSizeMin, DecalSizeMax);
		FVector SizeVector = UKismetMathLibrary::MakeVector(Size, Size, Size);
		float Rotation = UKismetMathLibrary::RandomFloatInRange(DecalRotationMin, DecalRotationMax);
		FRotator SizeRotator = UKismetMathLibrary::MakeRotator(Rotation, -90.f, Rotation);
		auto DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ImpactSurface.DecalMaterial, SizeVector, ImpactLocation, SizeRotator, DecalLifetime);
		DecalComponent->SetFadeOut(DecalLifetime, DecalFadeOutDuration);
	}

	if (CameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), CameraShake, ImpactLocation, CamShakeInnerRadius, CamShakeOuterRadius);
	}
}

void AProjectile::AlertNearbyActors()
{
	// check overlap detection when nearing to a stop
	if (!DetectNearbyActors || GetVelocity().Size() >= 5.f) {
		return;
	}

	/** Prevent processing the same overlapped actors */
	TArray<AActor*> DetectionActors;

	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(ExplosiveRadiusOuter);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Visibility, MyColSphere, GetQueryParams());

	if (isHit)
	{
		// loop through TArray
		for (auto& Hit : OutHits)
		{
			AActor* DamagedActor = Hit.GetActor();

			if (!DamagedActor) {
				continue;
			}

			if (DamagedActor == this) {
				continue;
			}

			if (DetectionActors.Contains(DamagedActor)) {
				continue;
			}

			if (!UHealthComponent::IsActorAlive(DamagedActor)) {
				continue;
			}

			// Does actor implement the avoidable interface?
			if (!DamagedActor->GetClass()->ImplementsInterface(UAvoidable::StaticClass())) {

				// Does the actor's controller implement the avoidable interface then?
				if (DamagedActor->GetInstigatorController() && !DamagedActor->GetInstigatorController()->GetClass()->ImplementsInterface(UAvoidable::StaticClass())) {
					continue;
				}
			}

			FAvoidableParams AvoidableParams;
			AvoidableParams.Actor = this;
			AvoidableParams.AvoidableDistance = ExplosiveRadiusOuter;

			// Does actor implement the avoidable interface?
			if (DamagedActor->GetClass()->ImplementsInterface(UAvoidable::StaticClass())) {
				IAvoidable::Execute_OnNearbyActorFound(DamagedActor, AvoidableParams);
			}

			// Does actor's controller implement the avoidable interface
			if (DamagedActor->GetInstigatorController() && DamagedActor->GetInstigatorController()->GetClass()->ImplementsInterface(UAvoidable::StaticClass())) {
				IAvoidable::Execute_OnNearbyActorFound(DamagedActor->GetInstigatorController(), AvoidableParams);
			}

			DetectionActors.Add(DamagedActor);
		}

	}
}

bool AProjectile::IsInAir()
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(this);

	FVector Start = GetActorLocation();
	FVector End = Start + FVector(.0f, .0f, -100.f);

	FHitResult OutHit;
	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);
}

void AProjectile::SetCountdownTimer(float Value)
{
	// stop the current countdown if counting down.
	GetWorldTimerManager().ClearTimer(THandler_CountdownTimer);

	CountdownTimer = Value;

	// start countdown again with new value.
	if (CountdownTimer > 0.f) {
		GetWorldTimerManager().SetTimer(THandler_CountdownTimer, this, &AProjectile::SelfDestruct, 1.f, true, CountdownTimer);
	}
}

void AProjectile::FindHomingTarget(AActor* TargetActor)
{
	if (TargetActor == nullptr || !UKismetSystemLibrary::IsValid(TargetActor)) {
		return;
	}

	// Use the projectile movement component homing function to make this work.
	auto ProjectileMovementComponent = Cast<UProjectileMovementComponent>(GetComponentByClass(UProjectileMovementComponent::StaticClass()));

	// no projectile movment comp?
	if (!ProjectileMovementComponent) {
		return;
	}

	USceneComponent* TargetComp = nullptr;

	// Some targets may have meshes that have been translated up or down from the root component such as aircraft, the root component of the aircraft can be below the mesh.
	// This assumes the skeletal mesh is the main body of the target.
	auto SkeletalComp = Cast<USkeletalMeshComponent>(TargetActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));

	if (SkeletalComp)
	{
		TargetComp = SkeletalComp;
	}


	// set the to any static mesh comps which the target actor may have if no skeletal mesh exists.
	auto StaticComp = Cast<UStaticMeshComponent>(TargetActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

	if (!TargetComp)
	{
		TargetComp = StaticComp;
	}

	// no target comp set? set to root component of actor.
	if (!TargetComp)
	{
		TargetComp = TargetActor->GetRootComponent();
	}

	// Remove previous target actor if present.
	if (HomingTargetActor) 
	{
		if (HomingTargetActor->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
		{
			ITargetable::Execute_OnMissileDestroyed(HomingTargetActor, this);
		}
	}

	HomingTargetActor = TargetActor;

	ProjectileMovementComponent->HomingTargetComponent = TargetComp;
	ProjectileMovementComponent->bIsHomingProjectile = true;


	if (TargetActor->GetClass()->ImplementsInterface(UTargetable::StaticClass())) {
		ITargetable::Execute_OnMissileIncoming(TargetActor, this);
	}
}