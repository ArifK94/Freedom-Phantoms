// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/HealthComponent.h"

#include "FreedomFighters/FreedomFighters.h"

#include "Characters/CombatCharacter.h"

#include "Kismet/KismetMathLibrary.h"

#include "PhysicalMaterials/PhysicalMaterial.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxHealth = 100;

	CanRegenerateHealth = true;
	RegenPerSecond = 10.0f;

	isAlive = false;

	HasUnlimitedHealth = false;
	HasTakenDamage = false;

	RegenerationDelayAmount = 5.0f;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	CurrentRegenerationDelay = RegenerationDelayAmount;
	isAlive = true;

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
		MyOwner->OnTakeRadialDamage.AddDynamic(this, &UHealthComponent::OnRadialDamage);
	}
}



void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (isAlive) {

		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

		mDeltaTime = DeltaTime;

		if (CanRegenerateHealth)
		{
			RegenerateHealth();
		}
	}
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	OnDamage(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);
}

void UHealthComponent::OnRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, FHitResult Hit, AController* InstigatedBy, AActor* DamageCauser)
{
	AActor* MyOwner = DamageCauser->GetOwner();

	OnDamage(DamagedActor, Damage, DamageType, InstigatedBy, MyOwner);
}

void UHealthComponent::OnDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (HasUnlimitedHealth) return;

	if (!isAlive) return;

	if (Damage <= 0.0f) return;

	if (DamageCauser != DamagedActor)
	{
		if (IsFriendly(DamagedActor, DamageCauser)) {
			return;
		}
	}

	// Update health clamp
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);


	// update the regeneration if taken damage as well as the delay time to wait again for another x seconds
	if (CanRegenerateHealth) {
		CurrentRegenerationDelay = RegenerationDelayAmount;
		HasTakenDamage = true;
	}


	if (Health <= 0.0f)
	{
		isAlive = false;


		// set death type
		EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitInfo.PhysMaterial.Get());

		switch (SurfaceType)
		{
		case SURFACE_HEAD:
			deathType = DeathType::Head;
			break;
		case SURFACE_FLESHVULNERABLE:
			deathType = DeathType::FleshVulnerable;
			break;
		case SURFACE_GROIN:
			deathType = DeathType::FleshVulnerable;
			break;
		default:
			deathType = DeathType::FleshDefault;
			break;
		}


		if (DamageCauser->IsA(ACombatCharacter::StaticClass()))
		{
			ACombatCharacter* opposingChar = Cast<ACombatCharacter>(DamageCauser);

			if (opposingChar)
			{
				opposingChar->EnemyKilled();
			}

			ACombatCharacter* currentActor = Cast<ACombatCharacter>(DamagedActor);

			if (currentActor)
			{
				auto nearestFriendly = currentActor->FindNearestFriendly();

				if (nearestFriendly)
				{
					nearestFriendly->FriendlyKilled();
				}
			}

		}
	}

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

void UHealthComponent::RegenerateHealth()
{
	if (HasTakenDamage)
	{
		// delay regeneration if more than 0
		if (CurrentRegenerationDelay > 0.0f)
		{
			CurrentRegenerationDelay -= mDeltaTime;
		}
		else // delay has finished countdown so ready to regenerate health
		{
			CurrentRegenerationDelay = RegenerationDelayAmount;
			HasTakenDamage = false;
		}
	}
	else // regenerating the health
	{
		if (Health < MaxHealth)
		{
			Health = FMath::Clamp(Health + RegenPerSecond * mDeltaTime, 0.0f, MaxHealth);

			OnHealthChanged.Broadcast(this, Health, 0, 0, 0, 0);
		}
	}
}


bool UHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	UHealthComponent* HealthCompA = Cast<UHealthComponent>(ActorA->GetComponentByClass(UHealthComponent::StaticClass()));
	UHealthComponent* HealthCompB = Cast<UHealthComponent>(ActorB->GetComponentByClass(UHealthComponent::StaticClass()));


	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	return HealthCompA->SelectedFaction == HealthCompB->SelectedFaction;
}


