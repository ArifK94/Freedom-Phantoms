// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/HealthComponent.h"

#include "Characters/CombatCharacter.h"

#include "Kismet/KismetMathLibrary.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TeamNumber = 255;

	MaxHealth = 100;

	CanRegenerateHealth = true;
	RegenPerSecond = 10.0f;

	isAlive = false;
}


void UHealthComponent::RegenerateHealth()
{
	if (Health < MaxHealth && isAlive)
	{
		Health++;
		//Health = FMath::Clamp((Health + RegenPerSecond) * mDeltaTime, 0.0f, MaxHealth);

		OnHealthChanged.Broadcast(this, Health, 0, 0, 0, 0);
	}
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	isAlive = true;

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	mDeltaTime = DeltaTime;

	RegenerateHealth();
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (!isAlive) return;

	if (Damage <= 0.0f) return;

	// return if self damage & if friendly fire
	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser))	return;
	


	// Update health clamp
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);


	if (Health <= 0.0f)
	{
		isAlive = false;
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

	return HealthCompA->TeamNumber == HealthCompB->TeamNumber;
}


