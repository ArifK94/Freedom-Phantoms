// Fill out your copyright notice in the Description page of Project Settings.


#include "Accessories/Accessory.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AAccessory::AAccessory()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAccessory::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAccessory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAccessory::CreateStaticMeshParent()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->CanCharacterStepUpOn = ECB_No;
}

void AAccessory::CreateSkeletalMeshParent()
{
	SkelMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SkelMesh->SetCollisionProfileName(TEXT("NoCollision"));
	SkelMesh->CanCharacterStepUpOn = ECB_No;
	SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
}

void AAccessory::setMeshSocket(UStaticMeshComponent* parentComp)
{
	StaticMesh->AttachToComponent(parentComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ParentSocket);
}

void AAccessory::setMeshSocket(USkeletalMeshComponent* parentComp)
{
	SkelMesh->AttachToComponent(parentComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ParentSocket);
}

UStaticMeshComponent* AAccessory::getMainStaticMesh()
{
	if (StaticMesh)
	{
		return StaticMesh;
	}

	return nullptr;
}

USkeletalMeshComponent* AAccessory::getMainSkelMesh()
{
	if (SkelMesh)
	{
		return SkelMesh;
	}

	return nullptr;
}
