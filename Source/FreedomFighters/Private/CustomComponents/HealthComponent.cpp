#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "FreedomFighters/FreedomFighters.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/Projectile.h"

#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxHealth = 100;

	CanRegenerateHealth = true;
	RegenPerSecond = 10.0f;

	isAlive = false;

	HasUnlimitedHealth = false;
	DefaulUnlimitedHealth = false;
	HasTakenDamage = false;
	IgnoreFriendlyFire = true;
	AcceptOnlyExplosions = false;
	ClearAllActorTimers = true;
	CanBeWounded = false;
	isWounded = false;

	RegenerationDelayAmount = 5.0f;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaulUnlimitedHealth = HasUnlimitedHealth;
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

void UHealthComponent::ApplyDamage(FHealthParameters HealthParameters)
{
	UHealthComponent* HealthComponent = Cast<UHealthComponent>(HealthParameters.DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

	if (HealthComponent)
	{
		HealthComponent->OnDamage(HealthParameters);
	}
}


void UHealthComponent::OnDamage(FHealthParameters HealthParameters)
{
	if (HasUnlimitedHealth) return;

	if (!isAlive) return;

	if (HealthParameters.Damage <= 0.0f) return;

	if (IgnoreFriendlyFire && !HealthParameters.CanDamageFriendlies)
	{
		if (HealthParameters.DamageCauser != HealthParameters.DamagedActor)
		{
			if (UTeamFactionComponent::IsFriendly(HealthParameters.DamagedActor, HealthParameters.DamageCauser)) {
				return;
			}
		}
	}

	if (AcceptOnlyExplosions)
	{
		if (!HealthParameters.IsExplosive)
		{
			return;
		}
	}


	auto DamageReduction = HealthParameters.Damage - DamageReduceFactor;

	// Update health clamp
	Health = FMath::Clamp(Health - DamageReduction, 0.0f, MaxHealth);


	// update the regeneration if taken damage as well as the delay time to wait again for another x seconds
	if (CanRegenerateHealth) {
		CurrentRegenerationDelay = RegenerationDelayAmount;
		HasTakenDamage = true;
	}


	if (Health <= 0.0f)
	{
		isAlive = false;

		if (ClearAllActorTimers)
		{
			GetWorld()->GetTimerManager().ClearAllTimersForObject(GetOwner());
		}

		if (CanBeWounded) {
			isWounded = true;
		}
	}


	HealthParameters.SetHealthComponent(this);
	OnHealthChanged.Broadcast(HealthParameters);
}


void UHealthComponent::RegenerateHealth()
{
	if (!isAlive) {
		return;
	}

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

			FHealthParameters HealthParameters;
			HealthParameters.SetHealthComponent(this);
			OnHealthChanged.Broadcast(HealthParameters);
		}
	}
}

void UHealthComponent::Revive()
{
	isAlive = true;
	isWounded = false;
	Health = 100.f;

	// Go back to default flag if has unlimited health
	HasUnlimitedHealth = DefaulUnlimitedHealth;
}

bool UHealthComponent::IsAlive(AActor* Owner)
{
	if (!Owner || Owner->GetName() == "None") {
		return false;
	}

	TWeakObjectPtr<AActor> TempActor = Owner;
	if (!TempActor.IsValid()) {
		return false;
	}

	auto ActorComponent = TempActor->GetComponentByClass(UHealthComponent::StaticClass());

	if (!ActorComponent) {
		return false;
	}

	auto HealthComponent = Cast<UHealthComponent>(ActorComponent);

	if (!HealthComponent) {
		return false;
	}

	return HealthComponent->IsAlive();
}

bool UHealthComponent::IsWounded(AActor* Owner)
{
	if (!Owner || Owner->GetName() == "None") {
		return false;
	}

	TWeakObjectPtr<AActor> TempActor = Owner;
	if (!TempActor.IsValid()) {
		return false;
	}

	auto ActorComponent = TempActor->GetComponentByClass(UHealthComponent::StaticClass());

	if (!ActorComponent) {
		return false;
	}

	auto HealthComponent = Cast<UHealthComponent>(ActorComponent);

	if (!HealthComponent) {
		return false;
	}

	return HealthComponent->GetIsWounded();
}

void UHealthComponent::SetIsReviving(AActor* Owner, bool Value)
{
	auto ActorComponent = Owner->GetComponentByClass(UHealthComponent::StaticClass());

	if (!ActorComponent) {
		return;
	}

	auto HealthComponent = Cast<UHealthComponent>(ActorComponent);

	if (!HealthComponent) {
		return;
	}

	// is in state of being revived
	if (Value) {
		HealthComponent->SetUnlimitedHealth(true);
		HealthComponent->SetIsAlive(true);
	}
	// has been revived
	else {
		HealthComponent->Revive();
	}

}
