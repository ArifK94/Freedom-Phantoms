#include "Weapons/AssaultRifle.h"

#include "Components/StaticMeshComponent.h"


AAssaultRifle::AAssaultRifle()
{
	BarrelComp = nullptr;

	weaponType = WeaponType::Rifle;
}


void AAssaultRifle::BeginPlay()
{
	Super::BeginPlay();

	setBarrel();
}

void AAssaultRifle::setBarrel()
{
	// Get Barrel Component
	for (UActorComponent* component : GetComponentsByTag(UStaticMeshComponent::StaticClass(), "Barrel"))
	{
		BarrelComp = Cast<UStaticMeshComponent>(component);
	}

	// If there is a barrel component, set the mesh
	if (BarrelComp)
	{
		BarrelComp->SetStaticMesh(BarrelMeshes[0]);
	}
}

FVector AAssaultRifle::getMuzzleLocation()
{
	if (BarrelComp)
	{
		return BarrelComp->GetSocketLocation(MuzzleSocket);
	}

	return Super::getMuzzleLocation();
}