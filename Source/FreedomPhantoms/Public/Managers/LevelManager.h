#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelManager.generated.h"

UCLASS()
class FREEDOMPHANTOMS_API ALevelManager : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (AllowPrivateAccess = "true"))
		bool IsNightTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (AllowPrivateAccess = "true"))
		TArray<AActor*> DayTimeObjects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (AllowPrivateAccess = "true"))
		TArray<AActor*> NightTimeObjects;

public:	
	ALevelManager();

private:
	void ToggleObjects();

protected:
	virtual void BeginPlay() override;

};
