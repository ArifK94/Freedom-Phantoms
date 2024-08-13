#include "Weapons/AmmoCrate.h"
#include "Weapons/Weapon.h"
#include "Weapons/ThrowableWeapon.h"
#include "Characters/CombatCharacter.h"

#include "Components/AudioComponent.h"

AAmmoCrate::AAmmoCrate()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = Mesh;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
}

void AAmmoCrate::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentHit.AddDynamic(this, &AAmmoCrate::OnCrateHit);
}

void AAmmoCrate::OnCrateHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == nullptr) {
		return;
	}

	ACombatCharacter* PlayerCharacter = Cast<ACombatCharacter>(OtherActor);

	if (PlayerCharacter == nullptr) {
		return;
	}

	// if character is controlled by player then ignore.
	if (!PlayerCharacter->IsPlayerControlled()) {
		return;
	}

	bool HasPrimaryReplenished = PlayerCharacter->GetPrimaryWeapon() && PlayerCharacter->GetPrimaryWeapon()->ReplenishAmmo();

	bool HasSecondaryReplenished = PlayerCharacter->GetSecondaryWeaponObj() && PlayerCharacter->GetSecondaryWeaponObj()->ReplenishAmmo();

	bool HasGrenadeReplenished = PlayerCharacter->GetGrenadeWeapon() && PlayerCharacter->GetGrenadeWeapon()->ReplenishAmmo();

	bool SuccessfulReplenish = HasPrimaryReplenished || HasSecondaryReplenished || HasGrenadeReplenished;

	if (SuccessfulReplenish)
	{
		PlaySuccess();
	}
	else
	{
		PlayFailed();
	}
}

void AAmmoCrate::PlaySuccess()
{
	if (ReplenishSuccessSound)
	{
		AudioComponent->Sound = ReplenishSuccessSound;
		AudioComponent->Play();		
	}
}

void AAmmoCrate::PlayFailed()
{
	if (ReplenishFailedSound)
	{
		AudioComponent->Sound = ReplenishFailedSound;
		AudioComponent->Play();
	}
}