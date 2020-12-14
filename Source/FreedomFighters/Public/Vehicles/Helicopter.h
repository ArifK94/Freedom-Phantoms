// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Helicopter.generated.h"

class ABaseCharacter;
UCLASS()
class FREEDOMFIGHTERS_API AHelicopter : public AActor
{
	GENERATED_BODY()
	
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* HelicopterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* RotorAudio;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Helicopter Sockets", meta = (AllowPrivateAccess = "true"))
		FName PassengerSeatPrefix;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Passengers", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ABaseCharacter> PassengerCharacter;
	 ABaseCharacter* PassengerCharacterObj;

	


public:	
	AHelicopter();

private:
	void SpawnPassenger();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
