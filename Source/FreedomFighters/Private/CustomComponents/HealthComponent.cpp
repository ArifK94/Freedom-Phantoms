#include "CustomComponents/HealthComponent.h"

#include "FreedomFighters/FreedomFighters.h"

#include "Characters/CombatCharacter.h"

#include "Kismet/KismetMathLibrary.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SelectedFaction = TeamFaction::Neutral;

	MaxHealth = 100;

	CanRegenerateHealth = true;
	RegenPerSecond = 10.0f;

	isAlive = false;

	HasUnlimitedHealth = false;
	HasTakenDamage = false;
	IgnoreFriendlyFire = true;
	AcceptOnlyExplosions = false;

	RegenerationDelayAmount = 5.0f;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	CurrentRegenerationDelay = RegenerationDelayAmount;
	isAlive = true;
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

void UHealthComponent::OnDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	if (HasUnlimitedHealth) return;

	if (!isAlive) return;

	if (Damage <= 0.0f) return;

	if (IgnoreFriendlyFire)
	{
		if (DamageCauser != DamagedActor)
		{
			if (IsFriendly(DamagedActor, DamageCauser)) {
				return;
			}
		}
	}

	if (AcceptOnlyExplosions)
	{
		if (!Bullet->IsExplosive())
		{
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
	}

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser, WeaponCauser, Bullet, HitInfo);
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

			FHitResult HitInfo;
			OnHealthChanged.Broadcast(this, Health, 0, 0, 0, 0, nullptr, nullptr, HitInfo);
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


