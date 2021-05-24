#pragma once

#include "CoreMinimal.h"
#include "Vehicles/Aircraft.h"
#include "AircraftTransport.generated.h"

class ARope;
UCLASS()
class FREEDOMFIGHTERS_API AAircraftTransport : public AAircraft
{
	GENERATED_BODY()
	

private:



private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	//void WaitForRapelling();

	//void UpdateOccupiedSeats();

public:


};
