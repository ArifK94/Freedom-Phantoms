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
#pragma region Ropes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName LeftRopeSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName RightRopeSocket;

	bool isLeftRappelOccupied;

	bool isRightRappelOccupied;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ARope> RopeClass;
	ARope* RopeLeft;
	ARope* RopeRight;

	FTimerHandle THandler_Rapelling;

#pragma endregion


private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void WaitForRapelling();

	void UpdateOccupiedSeats();

public:
	void IsLeftRappelOccupied(bool value) {
		isLeftRappelOccupied = value;
	}

	void IsRightRappelOccupied(bool value) {
		isRightRappelOccupied = value;
	}

};
