#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StructCollection.h"
#include "CountermeasureComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FREEDOMPHANTOMS_API UCountermeasureComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class AVehicleBase* Vehicle;

	UPROPERTY()
	AActor* SpawnedFlare;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> FlareClass;

	/** Number of countermeasures that can be released. -1 will mean it is infinite. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 TotalCounterLimit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 CurrentCounterTotal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float SpawnDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FCountermeasureParameters> CountermeasureParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USoundBase* SpawnSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USoundAttenuation* AttenuationSettings;

	/** 
	* Add delay when incoming threat is diverted. 
	* This allows a time gap between inbound and outbound threats to take place
	* otherwise the events will fire at the same time making the outbound threat may not work.
	*/


public:
	UCountermeasureComponent();

	UFUNCTION()
	void OnThreatUpdate(FIncomingThreatParameters IncomingThreatParams);

	/** Returns spawnedFlare. */
	void SpawnFlare(int32 Index, FCountermeasureParameters CountermeasureParam);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool CanSpawnFlare();

	/**
	* Set missile towards a spawned flare.
	*/
	void BeginThreatDivert(FIncomingThreatParameters IncomingThreatParams);


protected:
	virtual void BeginPlay() override;



};
