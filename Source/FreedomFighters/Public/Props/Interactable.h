#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

class UFactionManager;
class AAircraft;
class ABaseCharacter;
class APlayerController;
UCLASS()
class FREEDOMFIGHTERS_API AInteractable : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AAircraft> AircraftClass;
	AAircraft* Aircraft;

	/** Message to be displayed on the UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName ActionMessage;

	/** Is the Spawned Actor Controllable such as aircrafts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsControllable;
	
public:	
	AInteractable();

	void BeginInteraction(ABaseCharacter* Character, APlayerController* PlayerController);

	void SpawnAircraft(ABaseCharacter* Character, APlayerController* PlayerController);


protected:
	virtual void BeginPlay() override;

public:
	AAircraft* GetAircraft() {
		return Aircraft;
	}

	FName GetActionMessage() {
		return ActionMessage;
	}
};
