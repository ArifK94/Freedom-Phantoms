// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumCollection.h"
#include "Engine/DataTable.h"
#include "StructCollection.generated.h"

class AObjectPoolActor;
class ABaseCharacter;
class ACombatCharacter;
class ACommanderCharacter;
class AAircraft;
class ATargetSystemMarker;
class AWeapon;
class AMountedGun;
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
class UTexture;
class UTexture2D;
class ASupportPackage;
class UParticleSystem;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FMapDetail : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	/** The exact name of the map asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName LevelName;

	/** To be displayed on the loading screen or anywhere else */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UUserWidget> SatelliteWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* RenderTargetMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* SatelliteImage;

	/** Used for loading screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Thumbnail;

};

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FObjectPoolParameters
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
		TSubclassOf<AObjectPoolActor> PoolableActorClass;

	UPROPERTY()
		AObjectPoolActor* PoolableActor;

	UPROPERTY()
		int PoolSize;
};

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
struct FSupportPackageVoiceOverSet : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TeamFaction Faction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* AnnoucementReadToUse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* ReadyToUseSound;

	FSupportPackageVoiceOverSet()
	{
		AnnoucementReadToUse = nullptr;
		ReadyToUseSound = nullptr;
		Faction = TeamFaction::Neutral;
	}
};

USTRUCT(BlueprintType)
struct FSupportPackageSet : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<ASupportPackage> SupportActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AAircraft> AircraftClass;
	AAircraft* Aircraft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName Description;

	/** Message to be displayed on the UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName ActionMessage;

	/** Is the Spawned Actor Controllable such as aircrafts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsControllable;

	/** Icon displayed for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTexture* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTexture* PreviewImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* PickupSound;

	/** When user beings to interact with the object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* InteractSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<FSupportPackageVoiceOverSet> SupportSoundsSet;

	FSupportPackageSet()
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
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AWeapon>> AssaultRifles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AWeapon>> SMGs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AWeapon>> Shotguns;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AWeapon>> LMGs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AWeapon>> SecondaryWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AWeapon>> Handguns;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AWeapon>> MachinePistols;


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
		AircraftSpeedType AffectSpeedType;

	/** Aircraft path durations to be affected if AffectSpeedType is set to specified,
	* the higher the duration, the slower the aircraft speed (in seconds)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AircraftDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EAircraftMovement MovementType;
	
	/** Allow additional aircrafts to use the same spline path if current aircraft has finished its task rather than waiting for the current aircraft to reach its endpoint of the path  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsPathFreeToUse;
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

	/** FAircraftWeapon list index, leave as -1 meaning it does have a weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 AssociatedWeapon;

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
		OwningAircraft = nullptr;
		AssociatedWeapon = -1;
	}
};

USTRUCT(BlueprintType)
struct FAircraftWeapon
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<AMountedGun> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMax;

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

USTRUCT(BlueprintType)
struct FDeathAnimation : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> Defaults;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> Vulernables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> Headshots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> Groins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> Sprints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> Pistols;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> MountedGuns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> Crouches;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> ShotgunHitsFront;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> ShotgunHitsLegs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> ShotgunHitsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> ShotgunHitsRight;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> SprintExplosionsFront;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> SprintExplosionsBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> SprintExplosionsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> SprintExplosionsRight;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> StandExplosionsFront;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> StandExplosionsBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> StandExplosionsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UAnimSequence*> StandExplosionsRight;

};

USTRUCT(BlueprintType)
struct FSurfaceImpactSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UParticleSystem* ParticleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UNiagaraSystem* NiagaraEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundBase* Sound;

	FSurfaceImpactSet()
	{

	}
};


USTRUCT(BlueprintType)
struct FSurfaceImpact : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Concrete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Flesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Grass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Rock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Sand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Wood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FSurfaceImpactSet Water;

	FSurfaceImpact()
	{

	}
};

