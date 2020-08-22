// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/HealthComponent.h"

#include "Characters/CombatCharacter.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TeamNumber = 255;

	MaxHealth = 100;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}

}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f)
	{
		return;
	}

	// return if self damage & if friendly fire
	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}

	// Update health clamp
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	if (Health <= 0.0f)
	{
		if (DamageCauser->IsA(ACombatCharacter::StaticClass()))
		{
			ACombatCharacter* opposingChar = Cast<ACombatCharacter>(DamageCauser);

			if (opposingChar)
			{
				opposingChar->EnemyKilled();
			}


			//ACombatCharacter* friendlyChar = Cast<ACombatCharacter>(DamageCauser);

			//if (friendlyChar)
			//{
			//	friendlyChar->FriendlyKilled();
			//}

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


