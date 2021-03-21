#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectPoolActor.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API AObjectPoolActor : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
		float Lifespan;

	FTimerHandle THandler_LifespanTimer;
	bool isActive;
	
public:	
	AObjectPoolActor();

	void SetActive(bool InpActive);
	bool IsActive();
	virtual void Activate();
	void Deactivate();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
