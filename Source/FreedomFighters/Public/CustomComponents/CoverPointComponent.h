#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "CoverPointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UCoverPointComponent : public USphereComponent
{
	GENERATED_BODY()

private:
	AActor* Owner;

	FVector Location;
public:	
	UCoverPointComponent();

protected:
	virtual void BeginPlay() override;

public:	
	FVector GetLocation() {
		return Location;
	}

	AActor* GetOwner() {
		return Owner;
	}

	void SetOccupant(AActor* Actor) {
		Owner = Actor;
	}

		
};
