#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"


class ACombatCharacter;
class AAircraft;
class AGameHUDController;
UCLASS()
class FREEDOMFIGHTERS_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	APawn* OwningPawn;
	ACombatCharacter* OwningCombatCharacter;
	AGameHUDController* GameHUDController;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		AAircraft* ControlledAircraft;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float BaseLookUpRate;
	
private:
	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void BeginPlay() override;

	void AddControllerPitchInput(float Val);
	void AddControllerYawInput(float Val);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void SwitchWeapon();

	void BeginFire();



	void SpawnAC130();
};
