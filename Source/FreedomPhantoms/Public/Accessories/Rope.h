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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UCableComponent* CableComp;

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

	void UpdateCableLength();

	void DropRope();
	void ReleaseRope();

	void AttachActorToRope(AActor* Actor);
	void DettachActorToRope();

	FVector GetStartLocation();
	FVector GetEndLocation();

protected:
	virtual void BeginPlay() override;

public:	
	bool GetIsRopeOccupied() { return IsRopeOccupied; }
	void SetRopeOccupied(bool Value) { IsRopeOccupied = Value; }

	void SetRopeLeft(bool Value) { IsRopeLeft = Value; }

};
