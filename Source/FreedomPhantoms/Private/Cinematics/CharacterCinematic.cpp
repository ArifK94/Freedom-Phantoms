// Fill out your copyright notice in the Description page of Project Settings.


#include "Cinematics/CharacterCinematic.h"

#include "Components/SkeletalMeshComponent.h"

ACharacterCinematic::ACharacterCinematic()
{
	PrimaryActorTick.bCanEverTick = false;

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetCollisionProfileName(TEXT("NoCollision"));
	BodyMesh->CanCharacterStepUpOn = ECB_No;
	RootComponent = BodyMesh;

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetCollisionProfileName(TEXT("NoCollision"));
	HeadMesh->CanCharacterStepUpOn = ECB_No;
	HeadMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	LoaoutMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LoaoutMesh"));
	LoaoutMesh->SetCollisionProfileName(TEXT("NoCollision"));
	LoaoutMesh->CanCharacterStepUpOn = ECB_No;
	LoaoutMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	PrimaryWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PrimaryWeaponMesh"));
	PrimaryWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	PrimaryWeaponMesh->CanCharacterStepUpOn = ECB_No;
	PrimaryWeaponMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponHandSocket);

	HolsterWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HolsterWeaponMesh"));
	HolsterWeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	HolsterWeaponMesh->CanCharacterStepUpOn = ECB_No;
	HolsterWeaponMesh->AttachToComponent(LoaoutMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SidearmHolsterSocket);

	PrimaryWeaponScope = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PrimaryWeaponScope"));
	PrimaryWeaponScope->SetCollisionProfileName(TEXT("NoCollision"));
	PrimaryWeaponScope->CanCharacterStepUpOn = ECB_No;
	PrimaryWeaponScope->AttachToComponent(PrimaryWeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	HolsterWeaponScope = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HolsterWeaponScope"));
	HolsterWeaponScope->SetCollisionProfileName(TEXT("NoCollision"));
	HolsterWeaponScope->CanCharacterStepUpOn = ECB_No;
	HolsterWeaponScope->AttachToComponent(HolsterWeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	ShowPrimaryWeapon = true;
	ShowSecondaryWeapon = true;
	WeaponHandSocket = "weapon_hand";

	SidearmHolsterSocket = "sidearm_holster";
	BackHolsterSocket = "holster_back";
	Holster1Socket = "holster1";
	UseSidearmHolster = true;

	ACOGSocket = "tag_acog_2";
	EOTECHSocket = "tag_eotech";
	REDDOTSocket = "tag_red_dot";
	THERMALSocket = "tag_thermal_scope";

	FixTransform = true;
}

void ACharacterCinematic::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Update();
}

void ACharacterCinematic::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	Update();
}

void ACharacterCinematic::Update()
{
	HeadMesh->SetLeaderPoseComponent(BodyMesh);
	LoaoutMesh->SetLeaderPoseComponent(BodyMesh);

	SetWeaponHolster(PrimaryWeaponMesh, UsePrimarySidearmHolster, UsePrimaryBackHolster, UsePrimaryHolster, true);
	SetWeaponHolster(HolsterWeaponMesh, UseSidearmHolster, UseBackHolster, UseHolster1, false);

	ToggleAttachments(PrimaryWeaponMesh, PrimaryWeaponScope);

	PrimaryWeaponMesh->SetVisibility(ShowPrimaryWeapon, true);
	HolsterWeaponMesh->SetVisibility(ShowSecondaryWeapon, true);

	FixBodyTransform();
}

void ACharacterCinematic::ToggleAttachments(USkeletalMeshComponent* ParentWeapon, USkeletalMeshComponent* ScopeMeshComp)
{
	if (UseACOG)
	{
		ChangeWeaponAttachment(ParentWeapon, ScopeMeshComp, ACOGMesh, ACOGSocket);
	}
	else if (UseEOTECH)
	{
		ChangeWeaponAttachment(ParentWeapon, ScopeMeshComp, EOTECHMesh, EOTECHSocket);
	}
	else if (UseREDDOT)
	{
		ChangeWeaponAttachment(ParentWeapon, ScopeMeshComp, REDDOTMesh, REDDOTSocket);
	}
	else if (UseTHERMAL)
	{
		ChangeWeaponAttachment(ParentWeapon, ScopeMeshComp, THERMALMesh, THERMALSocket);
	}
	else
	{
		ScopeMeshComp->SetSkeletalMeshAsset(nullptr);
	}
}

void ACharacterCinematic::ChangeWeaponAttachment(USkeletalMeshComponent* ParentWeapon, USkeletalMeshComponent* ScopeMeshComp, USkeletalMesh* AttachmentMesh, FName Socket)
{
	ScopeMeshComp->SetSkeletalMeshAsset(AttachmentMesh);
	ScopeMeshComp->AttachToComponent(ParentWeapon, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
}

void ACharacterCinematic::SetWeaponHolster(USkeletalMeshComponent* ParentWeapon, bool CanUseSidearmHolster, bool CanUseBackHolster, bool CanUseHolster1, bool HoldWeapon)
{
	if (CanUseSidearmHolster)
	{
		ParentWeapon->AttachToComponent(LoaoutMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SidearmHolsterSocket);
	}
	else if (CanUseBackHolster)
	{
		ParentWeapon->AttachToComponent(LoaoutMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, BackHolsterSocket);
	}
	else if (CanUseHolster1)
	{
		ParentWeapon->AttachToComponent(LoaoutMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Holster1Socket);
	}
	else if (HoldWeapon)
	{
		ParentWeapon->AttachToComponent(BodyMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponHandSocket);
	}
}

void ACharacterCinematic::FixBodyTransform()
{
	if (!FixTransform)
	{
		FixTransform = true;
		BodyMesh->AddRelativeLocation(FVector(0.f, 0.f, -95.f));
		BodyMesh->AddRelativeRotation(FRotator(0.f, 0.f, -90.f));
	}
}
