
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
struct FREEDOMPHANTOMS_API FMapDetail : public FTableRowBase
{
	GENERATED_BODY()

public:

	/** To be displayed on the loading screen or anywhere else */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName DisplayName;

	/** The exact name of the map asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UUserWidget> SatelliteWidget;

	/** Image of the map's bird's eye view which is diplayed in the pause menu using the mapcamera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* RenderTargetMaterial;

	/**
	* Material of the map's bird's eye view which is diplayed in the deploying map screen.
	* Since the render target will be black/ blank when the map is not loaded, this material or image, will display the satellite view of the map.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterial* SatelliteImageryMaterial;

	/** Used for loading screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Thumbnail;

	FMapDetail()
	{
		RenderTargetMaterial = nullptr;
		SatelliteImageryMaterial = nullptr;
		Thumbnail = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FREEDOMPHANTOMS_API FObjectPoolParameters
{
	GENERATED_BODY()

public:
	UPROPERTY()
		TSubclassOf<AObjectPoolActor> PoolableActorClass;

	UPROPERTY()
		AObjectPoolActor* PoolableActor;

	UPROPERTY()
		int PoolSize;

	FObjectPoolParameters()
	{
		PoolableActorClass = nullptr;

		PoolableActor = nullptr;

		PoolSize = 0;
	}
};

USTRUCT(BlueprintType)
struct FREEDOMPHANTOMS_API FHealthParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UHealthComponent* AffectedHealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* DamagedActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* DamageCauser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		const class UDamageType* DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class AController* InstigatedBy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AWeapon* WeaponCauser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AProjectile* Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FHitResult HitInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsExplosive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ShouldWound;

	/**
	* Can damage friendlies? This override the health component's ignore friendly fire flag.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool CanDamageFriendlies;

	FHealthParameters()
	{
		AffectedHealthComponent = nullptr;
		DamagedActor = nullptr;
		DamageCauser = nullptr;
		DamageType = nullptr;
		InstigatedBy = nullptr;
		WeaponCauser = nullptr;
		Projectile = nullptr;
		Damage = .0f;
		IsExplosive = false;
		ShouldWound = false;
		CanDamageFriendlies = false;
	}

	void SetHealthComponent(UHealthComponent* InHealthComponent)
	{
		AffectedHealthComponent = InHealthComponent;
	}
};

USTRUCT(BlueprintType)
struct FREEDOMPHANTOMS_API FTargetSearchParameters
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
struct FREEDOMPHANTOMS_API FCoverSearchParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FTransform CvoerPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsCoverFound;

	FCoverSearchParameters()
	{
		IsCoverFound = false;
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
		USoundBase* OrderActionSuppressSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* OrderActionCoverMeSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* OrderMoveCombatSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* StayAlertSound;

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
		USoundBase* GrenadeThrowSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* GrenadeIncomingSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* StrongholdCaptureSound;

	FVoiceClipSet()
	{
		TargetFoundSound = nullptr;
		OrderActionSuppressSound = nullptr;
		OrderActionCoverMeSound = nullptr;
		OrderMoveCombatSound = nullptr;
		StayAlertSound = nullptr;
		ReloadingSound = nullptr;
		FriendlyDownSound = nullptr;
		EnemyDownSound = nullptr;
		RecruitSound = nullptr;
		AcknowledgeSound = nullptr;
		AcknowledgeCommandSound = nullptr;
		FollowSound = nullptr;
		AttackSound = nullptr;
		DefendSound = nullptr;
		RevivingSound = nullptr;
		RevivedSound = nullptr;
		DeathSound = nullptr;
		DeathFallSound = nullptr;
		GrenadeThrowSound = nullptr;
		GrenadeIncomingSound = nullptr;
		StrongholdCaptureSound = nullptr;
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
		AudioComponent = nullptr;
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
		Faction = TeamFaction::Neutral;
		AnnoucementReadToUse = nullptr;
		ReadyToUseSound = nullptr;
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
		SupportActorClass = NULL;
		VehicleClass = NULL;
		Vehicle = nullptr;
		IsControllable = false;
		Icon = nullptr;
		PreviewImage = nullptr;
		PickupSound = nullptr;
		InteractSound = nullptr;
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
		TArray<TSubclassOf<AWeapon>> RPGs;

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
struct FREEDOMPHANTOMS_API FWeaponUpdateParameters
{
	GENERATED_BODY()

public:
	/** Has the weapon fired a shot? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool HasFiredShot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EWeaponState WeaponState;


	FWeaponUpdateParameters()
	{
		WeaponState = EWeaponState::Default;
		HasFiredShot = false;
	}
};

USTRUCT(BlueprintType)
struct FREEDOMPHANTOMS_API FProjectileImpactParameters
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
		StandArmed = nullptr;
		CrouchArmed = nullptr;
		StandAimingBS = nullptr;
		StartMoveBS = nullptr;
		StartMoveCombatBS = nullptr;
		StopMoveBS = nullptr;
		CrouchAimingBS = nullptr;
		CrouchStartBS = nullptr;
		CrouchStopBS = nullptr;
		ProneAimingBS = nullptr;
		DrawMontage = nullptr;
		HolsterMontage = nullptr;
		Shooting = nullptr;
		Reloading = nullptr;
		AimOffsetStanding = nullptr;
		AimOffsetCrouching = nullptr;
		AimOffsetProning = nullptr;
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

	/** Remove player control from the aircraft if reached this point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool RemoveUserControl;


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

	FVehicleSplinePoint()
	{
		PointIndex = 0;
		PointLocation = FVector::ZeroVector;
		AffectSpeedType = EVehicleSpeedType::Normal;
		MovementType = EVehicleMovement::Grounded;
		PathDuration = 0.f;
		WaitingDuration = 0.f;
		IsPathFreeToUse = false;
		RemoveUserControl = false;
		CollisionBox = nullptr;
		CollisionSphere = nullptr;
		ArrowComponent = nullptr;
		TextRenderComponent = nullptr;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		ABaseCharacter* Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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

	/** If there is a door for this seat. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName SeatDoorSocketName;

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
		Role = EVehicleRole::SideGunner;

		IsSeatLeftSide = false;
		ExitPassengerOnPoint = false;

		PitchMin = 0.f;
		PitchMax = 0.f;
		YawMin = 0.f;
		YawMax = 0.f;
		CanCharacterShoot = 0.f;

		Character = nullptr;
		OwningVehicle = nullptr;
		IdleAnimation = nullptr;
		AimAnimation = nullptr;
		FireAnimation = nullptr;
		ExitAnimation = nullptr;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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

	/** Is this the main weapon? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool IsMainWeapon;

	/** The twin weapon set that is used to fire at the same time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<int> TwinWeaponIndexes;

	/** The socket name on the weapon which will allow the character to rotate with the turret, this is usually the socket that get rotated on the Yaw direction when moving th turret */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName WeaponAttachmentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UUserWidget> HUD;

	/** Class type which this weapon will focus to use against. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AActor>> PreferredClassTargets;

	/** Class type which this weapon will be used less against. e.g. tank main cannon will be less preferred to be used against infantry but sometimes can be used if tank gets more aggressive. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<TSubclassOf<AActor>> LessPreferredClassTargets;

	FVehicleWeapon()
	{
		AttachCharacterToWeapon = false;
		Weapon = nullptr;
		IsMainWeapon = false;
		PitchMin = 0.f;
		PitchMax = 0.f;
		YawMin = 0.f;
		YawMax = 0.f;
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
		YawValueMin = 0.f;
		YawValueMax = 0.f;
		NewPitchMin = 0.f;
		UseMinDefault = false;
		NewPitchMax = 0.f;
		UseMaxDefault = false;
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<ACombatCharacter> OperativeCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<ACommanderCharacter> CommanderCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UMaterialInterface* FlagMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicles")
		TArray<TSubclassOf<AVehicleBase>> GroundVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicles")
		TArray<TSubclassOf<AVehicleBase>> AirVehicles;

	FFaction()
	{
		FlagMaterial = nullptr;
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

	FDeathAnimation()
	{

	}
};

USTRUCT(BlueprintType)
struct FSurfaceImpactSet : public FTableRowBase
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

	/** Destruction sound attentuation  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundAttenuation* Attenuation;

	/** Add offset to the particle or niagara systems */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FTransform VFXOffset;

	FSurfaceImpactSet()
	{
		ParticleEffect = nullptr;
		NiagaraEffect = nullptr;
		AirParticleEffect = nullptr;
		AirNiagaraEffect = nullptr;
		DecalMaterial = nullptr;
		Sound = nullptr;
		Attenuation = nullptr;
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
struct FREEDOMPHANTOMS_API FStrongholdDefenderParams
{
	GENERATED_BODY()

public:
	/** The point which the defender should go to. */
	UPROPERTY()
		FVector TargetPoint;

	FStrongholdDefenderParams()
	{
		TargetPoint = FVector::ZeroVector;
	}
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FName FactionRowName;

	FFaction* FactionDataSet;

	FOccupiedFaction()
	{
		FactionCount = 0;
		Faction = TeamFaction::Neutral;
		FlagMaterial = nullptr;
		FactionDataSet = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FStrongholdDefender
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ACombatCharacter* CombatCharacter;

	FStrongholdDefender()
	{
		CombatCharacter = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FAvoidableParams
{
	GENERATED_BODY()

public:
	/**
	* The Actor to avoid
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* Actor;

	/**
	* The distance to avoid this actor
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float AvoidableDistance;

	FAvoidableParams()
	{
		Actor = nullptr;
		AvoidableDistance = .0f;
	}
};

USTRUCT(BlueprintType)
struct FChatableParams
{
	GENERATED_BODY()

public:
	/**
	* The Sound to play
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* Sound;

	/**
	* The delay amount until the sound to play.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PlayDelayTime;

	FChatableParams()
	{
		Sound = nullptr;
		PlayDelayTime = .0f;
	}
};

USTRUCT(BlueprintType)
struct FBattleChatterParams
{
	GENERATED_BODY()

public:
	/**
	* The Sound to play
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USoundBase* Sound;

	/**
	* The amount of seconds to cooldown.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CooldownAmount;

	/**
	* The time handler for cooldown before playing the sound again.
	*/
	UPROPERTY()
		FTimerHandle THandler_Cooldown;

	FBattleChatterParams()
	{
		Sound = nullptr;
		CooldownAmount = 0.f;
		THandler_Cooldown = FTimerHandle();
	}
};

USTRUCT(BlueprintType)
struct FWorldCoverPoint
{
	GENERATED_BODY()

public:
	UPROPERTY()
		AActor* Owner;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FVector Location;

	FWorldCoverPoint()
	{
		Owner = nullptr;
		Location = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FCharacterActionParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ECharacterAction Action;

	/** The height when character has landed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float LandHeight;

	FCharacterActionParameters()
	{
		Action = ECharacterAction::Crouch;
		LandHeight = 0.f;
	}
};

/**
* When homing missile for instance targets an actor.
*/
USTRUCT(BlueprintType)
struct FIncomingThreatParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AProjectile* Missile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 IncomingMissileCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bThreatDetected;

	FIncomingThreatParameters()
	{
		Missile = nullptr;
		IncomingMissileCount = 0;
		bThreatDetected = false;
	}
};

/**
* The params for countermeasuring threats
*/
USTRUCT(BlueprintType)
struct FCountermeasureParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName FlareSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 NumFlares;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Height;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TotalDegrees;


	FCountermeasureParameters()
	{
		FlareSocket = "FlareSocket";
		NumFlares = 1;
		SpawnRate = 1.f;
		Height = 0.f;
		Radius = 300.f;
		TotalDegrees = 360.f;
	}
};


USTRUCT(BlueprintType)
struct FRappellingParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsRappelling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsComplete;


	FRappellingParameters()
	{
		IsRappelling = false;
		IsComplete = false;
	}
};

USTRUCT(BlueprintType)
struct FCoverUpdateInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RightInputValue;

	FCoverUpdateInfo()
	{
		RightInputValue = 0.f;
	}
};