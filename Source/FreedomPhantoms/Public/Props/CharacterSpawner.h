// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterSpawner.generated.h"

class UCapsuleComponent;
class UArrowComponent;
UCLASS()
class FREEDOMPHANTOMS_API ACharacterSpawner : public AActor
{
	GENERATED_BODY()

private:
	/** The CapsuleComponent being used to align character to the ground. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UCapsuleComponent> CapsuleComponent;

#if WITH_EDITORONLY_DATA
	/** Component shown in the editor only to indicate character facing */
	UPROPERTY()
		TObjectPtr<UArrowComponent> ArrowComponent;
#endif

	/** Set the priority of spawn to prevent certain spawns from taking place. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int Priority;
	int DefaultPriority;

	
public:	
	ACharacterSpawner();

	UFUNCTION(BlueprintCallable)
		void SetDefaultPriority();

	/** Get character spawn by priority */
	UFUNCTION(BlueprintCallable)
		static bool GetSpawnTransform(UWorld* World, FVector& OutLocation, FRotator& OutRotation);

protected:
	virtual void BeginPlay() override;

public:
	int GetPriority() { return Priority; }
};
