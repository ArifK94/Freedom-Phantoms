// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Rope.generated.h"

class UPhysicsConstraintComponent;
UCLASS()
class FREEDOMPHANTOMS_API ARope : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* RopeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsRopeDeployed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsRopeReleased;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsRopeOccupied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsRopeLeft;

public:
	ARope();

	void DeployRope();
	void DetachRope();

	void AttachActorToRope(AActor* Actor);
	void DettachActorFromRope();

	// Function to get the bone's location along the rope
	FVector GetBoneLocation(int32 BoneIndex) const;

	// Function to get the total number of bones in the rope
	int32 GetNumBones() const;

public:

	bool GetIsRopeDeployed() { return IsRopeDeployed; }


	bool GetIsRopeOccupied() { return IsRopeOccupied; }
	void SetRopeOccupied(bool Value) { IsRopeOccupied = Value; }

	void SetRopeLeft(bool Value) { IsRopeLeft = Value; }

};
