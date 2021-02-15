#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetSystemMarker.generated.h"

class UWidgetComponent;
UCLASS()
class FREEDOMFIGHTERS_API ATargetSystemMarker : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UWidgetComponent* MarkerComponent;

public:	
	ATargetSystemMarker();


};
