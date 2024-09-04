#include "CustomComponents/CountermeasureComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "Vehicles/VehicleBase.h"
#include "Weapons/Projectile.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UCountermeasureComponent::UCountermeasureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	TotalCounterLimit = 2;
	CurrentCounterTotal = 0;
	SpawnDelay = 1.f;
}

void UCountermeasureComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		Vehicle = Cast<AVehicleBase>(GetOwner());

		if (Vehicle)
		{
			Vehicle->OnIncomingThreatUpdate.AddDynamic(this, &UCountermeasureComponent::OnThreatUpdate);
		}
	}

}

void UCountermeasureComponent::OnThreatUpdate(FIncomingThreatParameters IncomingThreatParams)
{
	if (IncomingThreatParams.bThreatDetected && CanSpawnFlare())
	{
		for (auto CParam : CountermeasureParams)
		{
			SpawnFlare(0, CParam);
		}
		CurrentCounterTotal++;


		FTimerHandle RemoveThreatTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(RemoveThreatTimerHandle, FTimerDelegate::CreateUObject(this, &UCountermeasureComponent::BeginThreatDivert, IncomingThreatParams), 1.f, false, SpawnDelay);
	}
}


void UCountermeasureComponent::SpawnFlare(int32 Index, FCountermeasureParameters CountermeasureParam)
{
	if (!FlareClass || !UHealthComponent::IsActorAlive(Vehicle))
	{
		return;
	}


	FVector SpawnLocation;
	FRotator SocketRotation;
	Vehicle->GetMeshComponent()->GetSocketWorldLocationAndRotation(CountermeasureParam.FlareSocket, SpawnLocation, SocketRotation);


	float AngleStep = CountermeasureParam.TotalDegrees / CountermeasureParam.NumFlares;
	float HeightStep = CountermeasureParam.Height / CountermeasureParam.NumFlares;

	float Angle = Index * AngleStep;
	float RadianAngle = FMath::DegreesToRadians(Angle);

	FVector Offset(
		FMath::Cos(RadianAngle) * CountermeasureParam.Radius,
		FMath::Sin(RadianAngle) * CountermeasureParam.Radius,
		Index * HeightStep
	);

	FVector FlareLocation = SpawnLocation + Offset;
	FRotator FlareRotation = FRotator(SocketRotation.Pitch, Angle + SocketRotation.Yaw, SocketRotation.Roll);


	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Vehicle;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnedFlare = GetWorld()->SpawnActor<AActor>(FlareClass, FlareLocation, FlareRotation, SpawnParams);

	if (SpawnedFlare)
	{
		if (SpawnSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SpawnSound, SpawnLocation, FlareRotation);
		}
	}

	// Schedule the next flare spawn if there are more to spawn
	if (Index + 1 < CountermeasureParam.NumFlares)
	{
		FTimerHandle NextFlareTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(NextFlareTimerHandle, FTimerDelegate::CreateUObject(this, &UCountermeasureComponent::SpawnFlare, Index + 1, CountermeasureParam), CountermeasureParam.SpawnRate, false);
	}
}

void UCountermeasureComponent::BeginThreatDivert(FIncomingThreatParameters IncomingThreatParams)
{
	if (SpawnedFlare && IncomingThreatParams.Missile)
	{
		IncomingThreatParams.Missile->FindHomingTarget(SpawnedFlare);
	}
}


bool UCountermeasureComponent::CanSpawnFlare()
{
	return CurrentCounterTotal < TotalCounterLimit;
}

