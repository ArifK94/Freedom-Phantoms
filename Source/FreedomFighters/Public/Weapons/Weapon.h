#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Engine/DataTable.h"

#include "Weapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class USoundBase;
class USoundCue;
class UAudioComponent;
class UArrowComponent;
class UAnimMontage;
class UBlendSpace;
class UBlendSpace1D;
class UAnimationAsset;
class UAnimMontage;
class UAnimSequence;
class UAimOffsetBlendSpace;
class AWeaponClip;
class AWeaponBullet;
class UObjectPoolComponent;

UENUM(BlueprintType)
enum class WeaponType : uint8
{
	Rifle		UMETA(DisplayName = "Rifle"),
	SMG 		UMETA(DisplayName = "SMG"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	LMG			UMETA(DisplayName = "LMG"),
	Pistol		UMETA(DisplayName = "Pistol")
};

UENUM(BlueprintType)
enum class SelectiveFire : uint8
{
	Automatic		UMETA(DisplayName = "Automatic"),
	SemiAutomatic 	UMETA(DisplayName = "SemiAutomatic"),
	Burst			UMETA(DisplayName = "Burst")
};

USTRUCT(BlueprintType)
struct FWeaponAnimSet : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		WeaponType weaponType;

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
struct FWeaponChargeSound : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundBase* Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool PlayOnLoop;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool HasPlayedSound;

	FWeaponChargeSound()
	{

	}
};


UCLASS()
class FREEDOMFIGHTERS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

	void StartFire();

	void StopFire();

	// Charging the weapon before being able to fire or used when holding the aim button eg. minigun aiming
	void ChargeUp();

	void ChargeDown();

	void IncreaseCharge();

	void DecreaseCharge();

	void BeginReload();

	void EndReload();

	void ClipIn();

	void ClipOut();

	void SetMagazineSocket();

	void SetClipSocket(USkeletalMeshComponent* meshComponent);

	void setWeaponSocket(USkeletalMeshComponent* meshComponent, FName socket);

	void SpawnWeaponAttachments();

	void SetHandGuardIK(USkeletalMeshComponent* CharacterMesh, FName TriggerHandSocket);

	/** Return bool so it can be used to play sounds if true */
	bool ReplenishAmmo();

private:
	void BurstDelay();
	void SemiFireDelay();

	void AutoReloadBegin();
	void AutoReloadEnd();

protected:
	virtual void Fire();

	void CreateBullet();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void OnReload();

	void ConfigSetup();

	void SpawnMagazine();

	void Recoil();

	virtual FVector getMuzzleLocation();

	void BeginShellEffect();

	void PlayShotEffect(FVector TracerEndPoint);

protected:
	int BurstAmmountCount;

	USkeletalMeshComponent* CharacterReference;

	FVector EyeLocation;
	FRotator EyeRotation;


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Hand Offsets", meta = (AllowPrivateAccess = "true"))
		USceneComponent* HandguardMesh;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Ammo", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeaponClip> weaponClip;
	AWeaponClip* weaponClipObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Ammo", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeaponBullet> BulletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		SelectiveFire selectiveFireMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UWeaponAttachmentManager> WeaponAttachmentClass;
	UWeaponAttachmentManager* WeaponAttachmentObj;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Ammo")
		bool canShowClip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool isFiring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool isReloading;
	bool IsAiming;


	/** Useful for cannons, as they may not have reload animation but instead a countdown reload time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool CanAutoReload;
	FTimerHandle THandler_AutoReloadBegin;
	FTimerHandle THandler_AutoReloadEnd;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		WeaponType weaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName MuzzleSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName TracerTargetSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName ClipSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName HolsterSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName ParentHolderSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName EjectorSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName HandguardSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName ReloadClipHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName OpticsSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName LaserSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName TorchlightSocket;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Hand Offsets", meta = (AllowPrivateAccess = "true"))
		FTransform HandguardOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		float CooldownReload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float RateOfFire;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Ammo")
		int32 CurrentAmmo;


	/** Current max which changes on reload */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Ammo")
		int32 CurrentMaxAmmo;

	/** Used for when it comes to replenishing and setting the original max ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Ammo")
		int32 MaxAmmoCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Ammo", meta = (AllowPrivateAccess = "true"))
		int32 AmmoPerClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ammo", meta = (AllowPrivateAccess = "true"))
		bool HasUnlimitedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Ammo", meta = (AllowPrivateAccess = "true"))
		bool HasNoReload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
		float ZoomFOV;


	/** RECOIL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Recoil", meta = (AllowPrivateAccess = "true"))
		bool hasRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
		float BulletSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Recoil")
		float RecoilAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Recoil")
		float HorizontalRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Recoil")
		float VerticleRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Recoil", meta = (AllowPrivateAccess = "true"))
		float TargetVerticalRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Recoil", meta = (AllowPrivateAccess = "true"))
		float TargetHorizontalRecoil;





	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ShotSound;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ReloadClipOutSound;
	bool HasPlayedClipOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ReloadClipInSound;
	bool HasPlayedClipIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ReloadEndSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* FireAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UObjectPoolComponent* ObjectPoolComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ShotAudioComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		bool PlayFireSoundAtLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Sounds", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ClipAudioComponent;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ShellEjectEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* TracerEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Muzzle")
		FVector CurrentMuzzleLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Muzzle")
		FRotator CurrentMuzzleRotation;

	/** Message to be displayed on the UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
		FName PickupMessage;


	// Derived from RateOfFire
	float TimeBetweenShots;
	float LastFireTime;
	FTimerHandle THandler_TimeBetweenShots;








	/** Charging Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Charging", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ChargingAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Charging")
		TArray<FWeaponChargeSound> ChargeUpSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Charging")
		TArray<FWeaponChargeSound> ChargeDownSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Charging")
		float ChargeUpTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Charging")
		bool IsChargingUp;

	float CurrentChargeUpTime;
	FTimerHandle THandler_ChargeUp;
	FTimerHandle THandler_ChargeDown;

private:
	float CurrentVerticleRecoil;

	USceneComponent* EyeViewPointComponent; // used for vehicles rather than pawns


protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:

	void setCharacter(USkeletalMeshComponent* mesh) { CharacterReference = mesh; }
	USkeletalMeshComponent* getCharacter() { return CharacterReference; }

	USkeletalMeshComponent* getMeshComp() { return MeshComp; }

	UWeaponAttachmentManager* getWeaponAttachmentObj() { return WeaponAttachmentObj; }


	int32 getCurrentAmmo() { return CurrentAmmo; }
	int32 getCurrentMaxAmmo() { return CurrentMaxAmmo; }
	int32 getAmmoPerClip() { return AmmoPerClip; }
	int32 GetMaxAmmoCapacity() { return MaxAmmoCapacity; }

		
	FName GetMuzzleSocket() { return MuzzleSocket; }
	FName getHolsterSocket() { return HolsterSocket; }
	FName getParentHolderSocket() { return ParentHolderSocket; }
	FName getOpticsSocket() { return OpticsSocket; }
	FName getLaserSocket() { return LaserSocket; }
	FName getTorchlightSocket() { return TorchlightSocket; }

	FName GetPickupMessage() {
		return PickupMessage;
	}

	bool getIsReloading() { return isReloading; }
	bool getIsFiring() { return isFiring; }

	bool GetHasUnlimitedAmmo() {
		return HasUnlimitedAmmo;
	}

	void SetIsAiming(bool Value) {
		IsAiming = Value;
	}

	void SetUnlimitedAmmo(bool IsUnlimited) {
		HasUnlimitedAmmo = IsUnlimited;
	}

	void SetComponentEyeViewPoint(USceneComponent* Comp) {
		EyeViewPointComponent = Comp;
	}


	float GetZoomFOV() {
		return ZoomFOV;
	}

	UAudioComponent* GetShotAudioComponent() {
		return ShotAudioComponent;
	}

	UAudioComponent* GetClipAudioComponent() {
		return ClipAudioComponent;
	}
};
