// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rope.generated.h"

class UPhysicsAsset;
UCLASS()
class FREEDOMFIGHTERS_API ARope : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* ropeMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPhysicsAsset* RopeDropPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPhysicsAsset* RopeRappelPhysics;

public:	
	ARope();

	void DropRope();

private:
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void CreateCollisionPoints();

protected:
	virtual void BeginPlay() override;

public:	
	USkeletalMeshComponent* RopeMeshComp() {
		return ropeMeshComp;
	}
};
