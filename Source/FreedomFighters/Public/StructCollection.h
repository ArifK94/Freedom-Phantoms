
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnumCollection.h"
#include "Engine/DataTable.h"
#include "StructCollection.generated.h"

class AObjectPoolActor;
class UHealthComponent;
class ABaseCharacter;
class ACombatCharacter;
class ACommanderCharacter;
class AVehicleBase;
class ATargetSystemMarker;
class AWeapon;
class AThrowableWeapon;
class AMountedGun;
class AProjectile;
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
	GENERATED_BODY()

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
	GENERATED_BODY()

public:
	UPROPERTY()
		TSubclassOf<AObjectPoolActor> PoolableActorClass;

	UPROPERTY()
		AObjectPoolActor* PoolableActor;

	UPROPERTY()
		int PoolSize;
};

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FHealthParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UHealthComponent* AffectedHealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* DamagedActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* DamageCauser;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		const class UDamageType* DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class AController* InstigatedBy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AWeapon* WeaponCauser;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AProjectile* Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FHitResult HitInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsExplosive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool ShouldWound;

	FHealthParameters()
	{
		DamagedActor = nullptr;

		DamageCauser = nullptr;

		DamageType = nullptr;

		InstigatedBy = nullptr;

		WeaponCauser = nullptr;

		Projectile = nullptr;

		Damage = .0f;

		IsExplosive = false;

		ShouldWound = false;
	}

	void SetHealthComponent(UHealthComponent* InHealthComponent)
	{
		AffectedHealthComponent = InHealthComponent;
	}
};

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FTargetSearchParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FVector TargetLocation;

	FTargetSearchParameters()
	{
		TargetActor = nullptr;
		TargetLocation = FVector::ZeroVector;
	}
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
		USoundBase* RevivingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* RevivedSound;

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
struct FCrossfadeAudio
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* Audio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float StartAmount;

	/** Should it run infinite amound of times */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsLoop;

	/** In case loop is set, this will be set when played first time then checked if has already been assigned to preventing playing again if no loop */
	UPROPERTY()
		UAudioComponent* AudioComponent;

	FCrossfadeAudio()
	{
		Audio = nullptr;
		StartAmount = 0.f;
		IsLoop = false;
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
		TSubclassOf<AVehicleBase> VehicleClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AVehicleBase* Vehicle;

	/** The actor tag name of the path to follow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName VehiclePathTagName;

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
		Vehicle = nullptr;
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
		TSubclassOf<ALoadout> AssaultLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<ALoadout> LMGLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<ALoadout> ShotgunLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<ALoadout> SMGLoadout;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<AThrowableWeapon> GrenadeClass;


	FWeaponsSet()
	{

	}
};


/**
* Used when weapon state is changed
*/
USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FWeaponUpdateParameters
{
	GENERATED_BODY()

public:
	UPROPERTY()
		bool IsFiring;

	/** Has the weapon fired a shot? */
	UPROPERTY()
		bool HasFiredShot;


	FWeaponUpdateParameters()
	{
		IsFiring = false;
		HasFiredShot = false;
	}
};

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FProjectileImpactParameters
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AProjectile* ProjectileActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int KillCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsSingleKill;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsDoubleKill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsMultiKill;

	FProjectileImpactParameters()
	{
		ProjectileActor = nullptr;

		KillCount = 0;
		
		IsSingleKill = false;
		
		IsDoubleKill = false;
		
		IsMultiKill = false;
	}

	void SetProjectileActor(AProjectile* InProjectileActor)
	{
		ProjectileActor = InProjectileActor;
	}
};


USTRUCT(BlueprintType)
struct FWeaponAnimSet : public FTableRowBase
{
	GENERATED_BODY()

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

	FWeaponAnimSet()
	{

	}
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
		EVehicleSpeedType AffectSpeedType;

	/** 
		Vehicle path durations to be affected if AffectSpeedType enum is set to specified,
		the higher the duration, the slower the aircraft speed (in seconds)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PathDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EVehicleMovement MovementType;
	
	/**
		Waiting on a path point. Affected when MovementType is set to Waiting.
		When duraction is over,  MovementType is set to MovementForward to continue the path
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float WaitingDuration;

	/** Allow additional aircrafts to use the same spline path if current aircraft has finished its task rather than waiting for the current aircraft to reach its endpoint of the path  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsPathFreeToUse;


	/** Allow vehicle to overlap the collision objects to indicate which point of the path they have reached  */
	UPROPERTY()
		class UBoxComponent* CollisionBox;

	/** Collision designs to make it easy visualise in the editor  */
	UPROPERTY()
		class USphereComponent* CollisionSphere;

	UPROPERTY()
		class UArrowComponent* ArrowComponent;

	UPROPERTY()
		class UTextRenderComponent* TextRenderComponent;

};

USTRUCT(BlueprintType)
struct FAircraftWeapon
{
	GENERATED_BODY()

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
struct FVehicletSeating
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		EVehicleRole Role;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<ABaseCharacter> CharacterClass;

	UPROPERTY()
		ABaseCharacter* Character;

	UPROPERTY()
		AVehicleBase* OwningVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimSequence* IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimSequence* AimAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimSequence* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimSequence* ExitAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName SeatingSocketName;

	/** FVehicleWeapon list index, leave as -1 meaning passenger seat does not have a weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 AssociatedWeapon;


	/** Used when exiting vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsSeatLeftSide;

	/** Should the passenger exit when vehicle reaches the exit passenger point along the vehicle path? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool ExitPassengerOnPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawMax;


	/** Can passenger shoot from this seat? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool CanCharacterShoot;

	FVehicletSeating()
	{
		Character = nullptr;
		OwningVehicle = nullptr;
		AssociatedWeapon = -1;
	}
};

USTRUCT(BlueprintType)
struct FVehicleWeapon
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<AMountedGun> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AMountedGun* Weapon;

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

	/** Allow the character to rotate in the Yaw direction with the MG */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool AttachCharacterToWeapon;

	/** The socket name on the weapon which will allow the character to rotate with the turret, this is usually the socket that get rotated on the Yaw direction when moving th turret */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponAttachmentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UUserWidget> HUD;

	FVehicleWeapon()
	{
		AttachCharacterToWeapon = false;
		Weapon = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FClampChangePitch
{
	GENERATED_BODY()

public:
	/** Change pitch min & max value when minimum Yaw value is...? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawValueMin;

	/** Change pitch min & max value when maximum Yaw value is...? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float YawValueMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float NewPitchMin;

	/** If no change needed, then this check this to true */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool UseMinDefault;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float NewPitchMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool UseMaxDefault;

	FClampChangePitch()
	{

	}
};

USTRUCT(BlueprintType)
struct FTargetSystemNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* Actor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ATargetSystemMarker* Marker; // the marker object containing the widget component

	FTargetSystemNode()
	{
		Actor = nullptr;
		Marker = nullptr;
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

	/** VFX for when in Air */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UParticleSystem* AirParticleEffect;

	/** VFX for when in Air */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UNiagaraSystem* AirNiagaraEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* DecalMaterial;

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

USTRUCT(BlueprintType)
struct FREEDOMFIGHTERS_API FStrongholdDefenderParams
{
	GENERATED_BODY()

public:
	/** The point which the defender should go to. */
	UPROPERTY()
		FVector TargetPoint;
};

USTRUCT(BlueprintType)
struct FOccupiedFaction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int FactionCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TeamFaction Faction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UMaterialInterface* FlagMaterial;

	FFaction* FactionDataSet;
};