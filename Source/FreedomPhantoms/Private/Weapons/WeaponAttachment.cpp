// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponAttachment.h"

#include "Components/SkeletalMeshComponent.h"

AWeaponAttachment::AWeaponAttachment()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->CanCharacterStepUpOn = ECB_No;
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

}

void AWeaponAttachment::SetDesertCamo()
{
	if (!DesertCamoMaterial) {
		return;
	}

	MeshComp->SetMaterial(BodyMaterialIndex, DesertCamoMaterial);
}