// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Goggle.generated.h"

UCLASS()
class FREEDOMPHANTOMS_API AGoggle : public AActor
{
	GENERATED_BODY()
	
public:	
	AGoggle();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goggles", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;



protected:
	virtual void BeginPlay() override;


public:
	UStaticMeshComponent* GetMesh() {
		return Mesh;
	}
};
