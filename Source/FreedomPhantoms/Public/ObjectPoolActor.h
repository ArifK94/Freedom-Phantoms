#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectPoolActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoolActorToggleSignature, AObjectPoolActor*, ObjectPoolActor, bool, IsActive);
UCLASS()
class FREEDOMPHANTOMS_API AObjectPoolActor : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
		float Lifespan;

	FTimerHandle THandler_LifespanTimer;
	bool isActive;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
		bool IsDestroyed;
	
public:	
	AObjectPoolActor();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
		FOnPoolActorToggleSignature OnPoolActorToggle;


	void SetActive(bool InpActive);
	bool IsActive();
	virtual void Activate();
	virtual void Deactivate();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;


	bool GetIsDestroyed() { return IsDestroyed; }
};
