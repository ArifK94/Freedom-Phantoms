// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/DestructionObjective.h"
#include "CustomComponents/HealthComponent.h"
#include "Weapons/Projectile.h"
#include "Services/SharedService.h"
#include "Managers/DatatableManager.h"

#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"

ADestructionObjective::ADestructionObjective()
{
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->Radius = 800.f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true; // ignore self

	CountdownTimer = 5.f;
	CountdownInterval = .1f;
	DestructionDamage = 500.f;

	ObjectiveMessage = "Begin Destruction";
	SurfaceImpactRowName = "Objective_Destruction";
}

void ADestructionObjective::BeginPlay()
{
	Super::BeginPlay();

	SurfaceImpactSet = UDatatableManager::RetrieveSurfaceImpactSet(GetWorld(), SurfaceImpactRowName);
}


FString ADestructionObjective::GetKeyDisplayName_Implementation()
{
	return FString();
}

FString ADestructionObjective::OnInteractionFound_Implementation(APawn* InPawn, AController* InController)
{
	return ObjectiveMessage.ToString();
}

AActor* ADestructionObjective::OnPickup_Implementation(APawn* InPawn, AController* InController)
{
	if (THandler_Countdown.IsValid()) {
		return nullptr;
	}

	if (DestructiveProjectileClass)
	{
		FHitResult OutHit;
		USharedService::IsInAir(OutHit, InPawn, 1000.f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		auto Projectile = GetWorld()->SpawnActor<AProjectile>(DestructiveProjectileClass, OutHit.ImpactPoint, FRotator::ZeroRotator, SpawnParams);

		if (Projectile)
		{
			Projectile->SetCountdownTimer(CountdownTimer);
		}
	}

	if (CountdownWidgetClass)
	{
		CountdownWidget = CreateWidget<UUserWidget>(GetWorld(), CountdownWidgetClass);

		if (CountdownWidget)
		{
			CountdownWidget->AddToViewport();
		}
	}

	CurrentCountdownTimer = CountdownTimer;
	GetWorldTimerManager().SetTimer(THandler_Countdown, this, &ADestructionObjective::BeginCountdown, CountdownInterval, true);

	OnObjectiveInteracted.Broadcast(this);

	return this;
}

bool ADestructionObjective::OnUseInteraction_Implementation(APawn* InPawn, AController* InController)
{
	return false;
}

bool ADestructionObjective::CanInteract_Implementation(APawn* InPawn, AController* InController)
{
	if (IsObjectiveComplete || THandler_Countdown.IsValid()) {
		return false;
	}

	return true;
}

void ADestructionObjective::OnDestruction()
{
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
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceImpactSet->Sound, GetActorLocation(), 1.0f, 1.0f, 0.0f, SurfaceImpactSet->Attenuation);
		}
	}

	// Blast away nearby physics actors
	RadialForceComp->FireImpulse();

	// Apply health damage
	ApplyExplosionDamage(GetActorLocation());


	Progress = 1.f;

	ObjectiveComplete();

}

void ADestructionObjective::BeginCountdown()
{
	// has countdown finished?
	if (CurrentCountdownTimer <= 0.f)
	{
		OnDestruction();

		CurrentCountdownTimer = 0.f;

		GetWorldTimerManager().ClearTimer(THandler_Countdown);
	}
	else
	{
		// Play sound when countdown has reached a whole number.
		auto RoundedCountdown = FMath::Fmod(CurrentCountdownTimer, 1);

		if (CountdownSound && RoundedCountdown < 0.1)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), CountdownSound);
		}
	}

	CurrentCountdownTimer -= CountdownInterval;

}

void ADestructionObjective::ApplyExplosionDamage(FVector ImpactPoint)
{
	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(RadialForceComp->Radius);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = GetWorld()->SweepMultiByChannel(OutHits, ImpactPoint, ImpactPoint, FQuat::Identity, ECC_Visibility, MyColSphere);

	TArray<AActor*> DamagedActors;

	if (isHit)
	{
		// loop through TArray
		for (auto& Hit : OutHits)
		{
			AActor* DamagedActor = Hit.GetActor();
			if (DamagedActor && !DamagedActors.Contains(DamagedActor))
			{
				DamagedActors.Add(DamagedActor);
			}
		}

		for (auto DamagedActor : DamagedActors)
		{
			UHealthComponent* DamagedHealthComponent = Cast<UHealthComponent>(DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

			if (DamagedHealthComponent)
			{
				FHealthParameters HealthParameters;
				HealthParameters.Damage = DestructionDamage;
				HealthParameters.DamagedActor = DamagedActor;
				HealthParameters.DamageCauser = this;
				HealthParameters.IsExplosive = true;
				DamagedHealthComponent->OnDamage(HealthParameters);
			}

			auto HitProjectile = Cast<AProjectile>(DamagedActor);
			if (HitProjectile && HitProjectile->IsExplosive()) {
				HitProjectile->SelfDestruct();
			}
		}
	}
}