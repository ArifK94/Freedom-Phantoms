#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverActor.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API ACoverActor : public AActor
{
	GENERATED_BODY()
	
private:

	UFUNCTION()
		void OnCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnCompEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:

	UPROPERTY(VisibleAnywhere)
		class USceneComponent* Root;

	/** The box component that informs the player if he's able to take cover or not */
	UPROPERTY(VisibleAnywhere)
		class UBoxComponent* BoxComp;

public:
	ACoverActor();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* SM;

};
