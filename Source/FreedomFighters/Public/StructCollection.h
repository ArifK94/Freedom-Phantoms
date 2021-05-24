// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumCollection.h"
#include "Engine/DataTable.h"
#include "StructCollection.generated.h"

class ABaseCharacter;
class ACombatCharacter;
class ACommanderCharacter;
class AAircraft;
class ATargetSystemMarker;
class AWeapon;
class UUserWidget;
class USoundBase;
class AHeadgear;
class ALoadout;
class UAnimMontage;
class UBlendSpace;
class UBlendSpace1D;
class UAnimationAsset;
class UAnimMontage;
class UAnimSequence;
class UAimOffsetBlendSpace;

USTRUCT(BlueprintType)
struct FVoiceClipSet : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* TargetFoundSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* ReloadingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* FriendlyDownSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* EnemyDownSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* RecruitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* AcknowledgeSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* AcknowledgeCommandSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* FollowSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* DefendSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* DeathFallSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* StrongholdCaptureSound;

	FVoiceClipSet()
	{

	}
};


USTRUCT(BlueprintType)
struct FAccessorySet : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<ALoadout>> Loadouts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf< AHeadgear>> Headgears;


	FAccessorySet()
	{

	}
};


USTRUCT(BlueprintType)
struct FWeaponsSet : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Assault")
		TArray<TSubclassOf<AWeapon>> AssaultRifles;

	UPROPERTY(EditAnywhere, Category = "SMG")
		TArray<TSubclassOf<AWeapon>> SMGs;

	UPROPERTY(EditAnywhere, Category = "Shotgun")
		TArray<TSubclassOf<AWeapon>> Shotguns;

	UPROPERTY(EditAnywhere, Category = "LMG")
		TArray<TSubclassOf<AWeapon>> LMGs;

	UPROPERTY(EditAnywhere, Category = "Secondary")
		TArray<TSubclassOf<AWeapon>> SecondaryWeapons;


	FWeaponsSet()
	{

	}
};

USTRUCT(BlueprintType)
struct FWeaponAnimSet : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace1D* StandArmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace1D* CrouchArmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* StandAimingBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* StartMoveBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* StartMoveCombatBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* StopMoveBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* CrouchAimingBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* CrouchStartBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* CrouchStopBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* ProneAimingBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* DrawMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* HolsterMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* Shooting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimMontage* Reloading;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAimOffsetBlendSpace* AimOffsetStanding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAimOffsetBlendSpace* AimOffsetCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAimOffsetBlendSpace* AimOffsetProning;
};



USTRUCT(BlueprintType)
struct FVehicleSplinePoint : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		int32 PointIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FVector PointLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EAircraftMovement MovementType;
};


USTRUCT(BlueprintType)
struct FAircraftSeating
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		EAircraftRole Role;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName SeatingSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 SeatPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 RappelAnimIndex;

	/** Check if rope is occupied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool isRopeLeftSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ABaseCharacter> Character;
	ABaseCharacter* CharacterObj;


	AAircraft* OwningAircraft;

	FAircraftSeating()
	{

	}
};

USTRUCT(BlueprintType)
struct FAircraftWeapon
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<AWeapon> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName CameraSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UUserWidget> HUD;

	FAircraftWeapon()
	{

	}
};

USTRUCT(BlueprintType)
struct FTargetSystemNode
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ABaseCharacter* Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		ATargetSystemMarker* Marker; // the marker class containing the widget component

	FTargetSystemNode()
	{

	}
};

USTRUCT(BlueprintType)
struct FFaction : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ACombatCharacter> OperativeCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ACommanderCharacter> CommanderCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AAircraft> AC130Class;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* FlagMaterial;

	FFaction()
	{

	}
};