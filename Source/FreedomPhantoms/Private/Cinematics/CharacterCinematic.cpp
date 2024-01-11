// Fill out your copyright notice in the Description page of Project Settings.


#include "Cinematics/CharacterCinematic.h"
#include "Weapons/Weapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/ChildActorComponent.h"
#include "Camera/CameraComponent.h"

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
	HeadMesh->SetupAttachment(RootComponent);

	LoaoutMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LoaoutMesh"));
	LoaoutMesh->SetCollisionProfileName(TEXT("NoCollision"));
	LoaoutMesh->CanCharacterStepUpOn = ECB_No;
	LoaoutMesh->SetupAttachment(RootComponent);


	PrimaryWeaponActorComp = CreateDefaultSubobject<UChildActorComponent>(FName(TEXT("PrimaryWeaponActorComp")));
	PrimaryWeaponActorComp->SetupAttachment(RootComponent);

	HolsterWeaponActorComp = CreateDefaultSubobject<UChildActorComponent>(FName(TEXT("HolsterWeaponActorComp")));
	HolsterWeaponActorComp->SetupAttachment(LoaoutMesh, SidearmHolsterSocket);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetHiddenInGame(false);

	UseFollowCameraViewPoiint = false;
	HeadSocket = "j_head";

	ShowPrimaryWeapon = true;
	ShowSecondaryWeapon = true;
	WeaponHandSocket = "weapon_hand";

	SidearmHolsterSocket = "sidearm_holster";
	BackHolsterSocket = "holster_back";
	Holster1Socket = "holster1";
	UseSidearmHolster = true;

	FixTransform = true;
}

void ACharacterCinematic::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Update();
}

#if WITH_EDITOR
void ACharacterCinematic::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	Update();
}
#endif

void ACharacterCinematic::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacterCinematic::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (UseFollowCameraViewPoiint && FollowCamera)
	{
		OutLocation = FollowCamera->GetComponentLocation();
		OutRotation = FollowCamera->GetComponentRotation();
	}
	else if (PrimaryWeaponActorComp->GetChildActor())
	{
		AWeapon* Weapon = Cast<AWeapon>(PrimaryWeaponActorComp->GetChildActor());

		if (Weapon)
		{
			Weapon->GetMeshComp()->GetSocketWorldLocationAndRotation(Weapon->GetMuzzleSocket(), OutLocation, OutRotation);
		}
	}
	else
	{
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
	}
}

void ACharacterCinematic::Update()
{
	HeadMesh->SetLeaderPoseComponent(BodyMesh);
	LoaoutMesh->SetLeaderPoseComponent(BodyMesh);


	PrimaryWeaponActorComp->SetVisibility(ShowPrimaryWeapon, true);
	HolsterWeaponActorComp->SetVisibility(ShowSecondaryWeapon, true);

	FollowCamera->AttachToComponent(BodyMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeadSocket);

	FixBodyTransform();

	if (PrimaryWeaponActorComp->GetChildActor())
	{
		AWeapon* Weapon = Cast<AWeapon>(PrimaryWeaponActorComp->GetChildActor());

		Weapon->SetOwner(this);
		Weapon->SetCrosshairErrorTolerance(0.f);
		AttachWeaponHolster(Weapon, UsePrimarySidearmHolster, UsePrimaryBackHolster, UsePrimaryHolster, true);
	}

	if (HolsterWeaponActorComp->GetChildActor())
	{
		AWeapon* Weapon = Cast<AWeapon>(HolsterWeaponActorComp->GetChildActor());

		Weapon->SetOwner(this);
		Weapon->SetCrosshairErrorTolerance(0.f);
		AttachWeaponHolster(Weapon, UseSidearmHolster, UseBackHolster, UseHolster1, false);
	}
}

void ACharacterCinematic::AttachWeaponHolster(AActor* ParentWeapon, bool CanUseSidearmHolster, bool CanUseBackHolster, bool CanUseHolster1, bool HoldWeapon)
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
		AttachWeaponHand(ParentWeapon, WeaponHandSocket);
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


void ACharacterCinematic::AttachWeaponHand(AActor* ParentWeapon, FName NewHandSocket)
{
	ParentWeapon->AttachToComponent(BodyMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NewHandSocket);
}
