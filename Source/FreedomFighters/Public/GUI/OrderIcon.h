// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrderIcon.generated.h"

UCLASS()
class FREEDOMFIGHTERS_API AOrderIcon : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Floor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Head;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		bool CanAnimate;

	/** Anything less or equal to 0.0f will be considered as unlimited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		float DisplayCountDown;

private:
	FTimerHandle THandler_Countdown;

	FVector OrginalPos;


public:	
	AOrderIcon();

	void SetRotation(AActor* TargetActor);

	void ShowIcon(FVector Location);
	void ShowIcon();
	void HideIcon();

private:
	void BeginCountDown();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
