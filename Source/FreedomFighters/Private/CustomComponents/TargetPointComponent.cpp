// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/TargetPointComponent.h"
#include "CustomComponents/HealthComponent.h"

UTargetPointComponent::UTargetPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UTargetPointComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UTargetPointComponent::OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser, AWeapon* WeaponCauser, AWeaponBullet* Bullet, FHitResult HitInfo)
{
	if (Health <= 0.f)
	{
		HealthComp->OnHealthChanged.RemoveDynamic(this, &UTargetPointComponent::OnHealthChanged);
		IsPointTaken = false;
	}
}

void UTargetPointComponent::SetPoint(AActor* Owner)
{
	if (!Owner) {
		return;
	}

	HealthComp = Cast<UHealthComponent>(Owner->GetComponentByClass(UHealthComponent::StaticClass()));

	if (HealthComp)
	{
		HealthComp->OnHealthChanged.AddDynamic(this, &UTargetPointComponent::OnHealthChanged);
		IsPointTaken = true;
	}
}