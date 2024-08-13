// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rope.generated.h"

UCLASS()
class FREEDOMPHANTOMS_API ARope : public AActor
{
	GENERATED_BODY()
	
private:
	FTimerHandle THandler_Destroy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsRopeDropped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsRopeReleased;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsRopeOccupied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsRopeLeft;

	/** The socket where characters will be holding onto the rope from the top part of the rope */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName TopAttachPointSocket;

public:	
	ARope();

	void DropRope();
	void ReleaseRope();

	void AttachActorToRope(AActor* Actor);
	void DettachActorToRope();

private:
	void DestroyRope();

public:	
	bool GetIsRopeOccupied() { return IsRopeOccupied; }
	void SetRopeOccupied(bool Value) { IsRopeOccupied = Value; }

	void SetRopeLeft(bool Value) { IsRopeLeft = Value; }

};
