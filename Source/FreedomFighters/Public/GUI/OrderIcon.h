#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/TimelineComponent.h"

#include "OrderIcon.generated.h"

class UCurveFloat;
UCLASS()
class FREEDOMFIGHTERS_API AOrderIcon : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Floor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Head;

	/** Anything less or equal to 0.0f will be considered as unlimited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DisplayCountDown;

	FTimeline CurveTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetPosAmountZ;

private:
	FTimerHandle THandler_Countdown;

	FVector OrginalPos;
	FVector TargetPos;

public:	
	AOrderIcon();

	void SetRotation(AActor* TargetActor);

	void ShowIcon(FVector Location);
	void ShowIcon();
	void HideIcon();

private:
	void BeginCountDown();

	UFUNCTION()
		void BeginAnimation(float Value);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
