// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrderIcon.generated.h"

class UMaterialInterface;

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
		UMaterialInterface* AttackMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commander Order Icon", meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* DefendMaterial;

private:
	FTimerHandle THandler_Countdown;

	FVector OrginalPos;


public:	
	AOrderIcon();

	void SetRotation(AActor* TargetActor);

	void ShowIcon(FVector Location);
	void HideIcon();

	void SetIconMaterial(UMaterialInterface* Material);

private:
	void BeginCountDown();
	void AnimateTransform();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UMaterialInterface* getAttackMaterial()
	{
		return AttackMaterial;
	}

	UMaterialInterface* getDefendMaterial()
	{
		return DefendMaterial;
	}
};
