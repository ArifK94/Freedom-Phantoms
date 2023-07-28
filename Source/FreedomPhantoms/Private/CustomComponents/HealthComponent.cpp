#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "FreedomPhantoms/FreedomPhantoms.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/Weapon.h"
#include "Weapons/Projectile.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100;
	isAlive = true;

	HasUnlimitedHealth = false;
	DefaulUnlimitedHealth = false;
	IgnoreFriendlyFire = true;
	AcceptOnlyExplosions = false;
	ClearAllActorTimers = true;
	CanBeWounded = false;
	isWounded = false;

	CanRegenerateHealth = true;
	RegenPerSecond = 10.0f;
	RegenerationDelayAmount = 5.0f;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaulUnlimitedHealth = HasUnlimitedHealth;
	CanBeWoundedDefault = CanBeWounded;
	Health = MaxHealth;
}

void UHealthComponent::ApplyDamage(FHealthParameters HealthParameters)
{
	if (!UKismetSystemLibrary::IsValid(HealthParameters.DamagedActor)) {
		return;
	}

	UHealthComponent* HealthComponent = Cast<UHealthComponent>(HealthParameters.DamagedActor->GetComponentByClass(UHealthComponent::StaticClass()));

	if (HealthComponent)
	{
		HealthComponent->OnDamage(HealthParameters);
	}
}

void UHealthComponent::ApplyExplosionDamage(AActor* DamageCauser, FVector ImpactPoint, float Radius, float Damage)
{
	// create a collision sphere
	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(Radius);

	// create tarray for hit results
	TArray<FHitResult> OutHits;

	// check if something got hit in the sweep
	bool isHit = DamageCauser->GetWorld()->SweepMultiByChannel(OutHits, ImpactPoint, ImpactPoint, FQuat::Identity, ECC_Visibility, MyColSphere);

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
				HealthParameters.Damage = Damage;
				HealthParameters.DamagedActor = DamagedActor;
				HealthParameters.DamageCauser = DamageCauser;
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

void UHealthComponent::SetCanBeWounded(bool Value)
{
	CanBeWounded = Value;

	if (!isAlive)
	{
		isWounded = CanBeWounded;
	}
}


void UHealthComponent::OnDamage(FHealthParameters HealthParameters)
{
	if (HasUnlimitedHealth) {
		return;
	}

	if (!isAlive) {
		return;
	}

	if (HealthParameters.DamageCauser == nullptr) {
		return;
	}

	if (HealthParameters.DamagedActor == nullptr) {
		return;
	}

	if (HealthParameters.Damage <= 0.0f) {
		return;
	}

	if (IgnoreFriendlyFire)
	{
		if (HealthParameters.DamageCauser != HealthParameters.DamagedActor)
		{
			// are both actors friendlies?
			// Does the health param override the damage to friendlies? If not, then ignore this damage.
			if (UTeamFactionComponent::IsFriendly(HealthParameters.DamagedActor, HealthParameters.DamageCauser) && !HealthParameters.CanDamageFriendlies) {
				return;
			}
		}
	}

	if (AcceptOnlyExplosions)
	{
		if (!HealthParameters.IsExplosive) {
			return;
		}
	}

	// return function if any immunue actors should be ignored.
	for (auto ActorClass : ImmuneActorClasses)
	{
		if (HealthParameters.Projectile && HealthParameters.Projectile->IsA(ActorClass))
		{
			return;
		}

		if (HealthParameters.DamageCauser && HealthParameters.DamageCauser->IsA(ActorClass))
		{
			return;
		}

		if (HealthParameters.WeaponCauser && HealthParameters.WeaponCauser->IsA(ActorClass))
		{
			return;
		}
	}

	GetOwner()->GetWorldTimerManager().ClearTimer(THandler_Regeneration);

	auto DamageReduction = HealthParameters.Damage - (HealthParameters.DamageCauser == HealthParameters.DamagedActor ? 0 : DamageReduceFactor);

	// Update health clamp
	Health = FMath::Clamp(Health - DamageReduction, 0.0f, MaxHealth);

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
	else
	{
		// set the regeneration timer if taken damage as well as the delay time to wait again for another x seconds
		if (CanRegenerateHealth) 
		{
			GetOwner()->GetWorldTimerManager().SetTimer(THandler_Regeneration, this, &UHealthComponent::RegenerateHealth, 1.f, true, RegenerationDelayAmount);
		}
	}


	HealthParameters.SetHealthComponent(this);
	OnHealthChanged.Broadcast(HealthParameters);
}


void UHealthComponent::RegenerateHealth()
{
	if (!isAlive || !GetOwner()) {
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_Regeneration);
		return;
	}

	if (Health < MaxHealth)
	{
		Health = FMath::Clamp(Health + RegenPerSecond, 0.0f, MaxHealth);

		FHealthParameters HealthParameters;
		HealthParameters.SetHealthComponent(this);
		OnHealthChanged.Broadcast(HealthParameters);
	}
	else
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_Regeneration);
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

bool UHealthComponent::IsActorAlive(AActor* Owner)
{
	if (!UKismetSystemLibrary::IsValid(Owner)) {
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

bool UHealthComponent::IsActorWounded(AActor* Owner)
{
	if (!UKismetSystemLibrary::IsValid(Owner)) {
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
