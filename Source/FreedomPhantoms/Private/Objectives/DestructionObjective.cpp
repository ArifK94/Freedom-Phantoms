// Fill out your copyright notice in the Description page of Project Settings.


#include "Objectives/DestructionObjective.h"
#include "CustomComponents/HealthComponent.h"

#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"

ADestructionObjective::ADestructionObjective()
{
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->Radius = 800.f;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true; // ignore self

	CountdownTimer = 5.f;
	DestructionDamage = 500.f;

	ObjectiveMessage = "Begin Destruction";
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

	CurrentCountdownTimer = CountdownTimer;
	GetWorldTimerManager().SetTimer(THandler_Countdown, this, &ADestructionObjective::BeginCountdown, 1.f, true, 0.f);

	return this;
}

bool ADestructionObjective::OnUseInteraction_Implementation(APawn* InPawn, AController* InController)
{
	return true;
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
	// Play FX & change self material
	if (DestructionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestructionEffect, GetActorLocation());
	}

	// Play explosion sound
	if (DestructionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestructionSound, GetActorLocation(), 1.0f, 1.0f, 0.0f, DestructionAttenuation);
	}

	// Blast away nearby physics actors
	RadialForceComp->FireImpulse();

	// Apply health damage
	ApplyExplosionDamage(GetActorLocation());


	Progress = 1.f;
	IsObjectiveComplete = true;

	ObjectiveComplete();

}

void ADestructionObjective::BeginCountdown()
{
	CurrentCountdownTimer -= 1.f;

	// has countdown finished?
	if (CurrentCountdownTimer <= 0.f)
	{
		OnDestruction();

		CurrentCountdownTimer = 0.f;

		GetWorldTimerManager().ClearTimer(THandler_Countdown);
	}
	else
	{
		if (CountdownSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), CountdownSound);
		}
	}
}

void ADestructionObjective::ApplyExplosionDamage(FVector ImpactPoint)
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
				UHealthComponent* DamagedHealthComponent = Cast<UHealthComponent>(DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

				if (DamagedHealthComponent && DamagedHealthComponent->IsAlive())
				{
					FHealthParameters HealthParameters;
					HealthParameters.Damage = DestructionDamage;
					HealthParameters.DamagedActor = DamagedActor;
					HealthParameters.DamageCauser = this;
					HealthParameters.IsExplosive = true;
					DamagedHealthComponent->OnDamage(HealthParameters);
				}
			}
		}
	}
}