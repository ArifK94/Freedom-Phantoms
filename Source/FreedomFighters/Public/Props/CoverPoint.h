#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverPoint.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API ACoverPoint : public AActor
{
	GENERATED_BODY()

private:
	AActor* Owner;

	UPROPERTY(EditAnywhere)
		FVector Location;

	UPROPERTY(EditAnywhere)
		bool ConvertToWorldSpace;
	
public:	
	ACoverPoint();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
