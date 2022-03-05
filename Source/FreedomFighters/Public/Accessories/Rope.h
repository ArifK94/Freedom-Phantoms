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
	FTimerHandle THandler_Destroy;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPhysicsAsset* RopeDropPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPhysicsAsset* RopeRappelPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsRopeDropped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsRopeReleased;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsRopeOccupied;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsRopeLeft;


	/** The socket where characters will be holding onto the rope from the top part of the rope */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FName TopAttachPointSocket;

public:	
	ARope();

	void DropRope();
	void ReleaseRope();

	void AttachActorToRope(AActor* Actor);

private:
	void DestroyRope();

protected:
	virtual void BeginPlay() override;

public:	
	bool GetIsRopeOccupied() { return IsRopeOccupied; }
	void SetRopeOccupied(bool Value) { IsRopeOccupied = Value; }

	void SetRopeLeft(bool Value) { IsRopeLeft = Value; }

};
