// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Accessory.generated.h"

class USkeletalMeshComponent;
UCLASS()
class FREEDOMPHANTOMS_API AAccessory : public AActor
{
	GENERATED_BODY()
	
public:	
	AAccessory();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* SkelMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Accessory", meta = (AllowPrivateAccess = "true"))
		FName ParentSocket;

public:
	void setMeshSocket(UStaticMeshComponent* parentComp);
	void setMeshSocket(USkeletalMeshComponent* parentComp);

	UStaticMeshComponent* getMainStaticMesh();
	USkeletalMeshComponent* getMainSkelMesh();

protected:
	void CreateStaticMeshParent();
	void CreateSkeletalMeshParent();




};
