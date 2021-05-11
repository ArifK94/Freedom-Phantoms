#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class USoundBase;
class USoundCue;
class UAudioComponent;
class AWeaponClip;
class AWeaponBullet;
class UObjectPoolComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmptyAmmoClipSignature, AWeapon*, Weapon);

UCLASS()
class FREEDOMFIGHTERS_API AWeapon : public AActor
{
	GENERATED_BODY()

private:
	FTimerHandle THandler_BulletSpread;


protected:
	int BurstAmmountCount;

	USkeletalMeshComponent* CharacterReference;


	// Derived from RateOfFire
	float TimeBetweenShots;
	float LastFireTime;
	FTimerHandle THandler_TimeBetweenShots;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* HandguardMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UObjectPoolComponent* ObjectPoolComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeaponClip> weaponClip;
	AWeaponClip* weaponClipObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeaponBullet> BulletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		SelectiveFire selectiveFireMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UWeaponAttachmentManager> WeaponAttachmentClass;
	UWeaponAttachmentManager* WeaponAttachmentObj;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
		bool canShowClip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool isFiring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool isReloading;
	bool IsAiming;


	/** Useful for cannons, as they may not have reload animation but instead a countdown reload time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanAutoReload;
	FTimerHandle THandler_AutoReloadBegin;
	FTimerHandle THandler_AutoReloadEnd;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		WeaponType weaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName MuzzleSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName ClipSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName HolsterSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName ParentHolderSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName EjectorSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName HandguardSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName ReloadClipHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName OpticsSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName LaserSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (AllowPrivateAccess = "true"))
		FName TorchlightSocket;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		FTransform HandguardOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float CooldownReload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float RateOfFire;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
		int32 CurrentAmmo;


	/** Current max which changes on reload */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
		int32 CurrentMaxAmmo;

	/** Used for when it comes to replenishing and setting the original max ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
		int32 MaxAmmoCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		int32 AmmoPerClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true", ClampMin = 1.0f))
		int32 BulletsPerFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		bool HasUnlimitedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		bool HasNoReload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
		float ZoomFOV;


	/** RECOIL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (AllowPrivateAccess = "true"))
		bool hasRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f, ClampMax = 1.0f))
		float BulletSpreadMin;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f, ClampMax = 1.0f))
		float BulletSpreadMax;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.1f))
		float BulletSpreadReduceRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Recoil")
		float BulletSpreadCurrent;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
		USoundBase* ShotSound;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
		USoundBase* ReloadClipOutSound;
	bool HasPlayedClipOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
		USoundBase* ReloadClipInSound;
	bool HasPlayedClipIn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
		USoundBase* ReloadEndSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* FireAttenuation;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ShotAudioComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		bool PlayFireSoundAtLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ClipAudioComponent;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ShellEjectEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* MuzzleEffect;

	/** Message to be displayed on the UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
		FName PickupMessage;


	/** Charging Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charging", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ChargingAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charging")
		USoundBase* ChargeUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charging")
		USoundBase* ChargeDownSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charging")
		FName ChargeSoundParamName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charging")
		float ChargeUpTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charging")
		bool IsChargingUp;

	float CurrentChargeUpTime;
	FTimerHandle THandler_ChargeUp;
	FTimerHandle THandler_ChargeDown;


	/** Debug */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		bool DrawShotLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float ShotLineDuration;

private:
	float CurrentVerticleRecoil;

	USceneComponent* EyeViewPointComponent; // used for vehicles rather than pawns


public:
	AWeapon();

	UPROPERTY(BlueprintAssignable)
		FOnEmptyAmmoClipSignature OnEmptyAmmoClip;

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

	virtual void SetIsAiming(bool isAiming);

private:
	void ConvertWeaponName();

	void BurstDelay();
	void SemiFireDelay();

	void AutoReloadBegin();
	void AutoReloadEnd();

	void ReduceBulletSpread();

	FVector RandomPointInCircle(float Radius);

protected:
	virtual void BeginPlay() override;

	virtual void Fire();

	void CreateBullet();

	UFUNCTION(BlueprintCallable)
		virtual void OnReload();

	void ConfigSetup();

	void SpawnMagazine();

	virtual FVector getMuzzleLocation();

	void BeginShellEffect();

	void PlayShotEffect(FVector EyeLocation);

public:

	void setCharacter(USkeletalMeshComponent* mesh) { CharacterReference = mesh; }
	USkeletalMeshComponent* getCharacter() { return CharacterReference; }

	USkeletalMeshComponent* getMeshComp() { return MeshComp; }

	UWeaponAttachmentManager* getWeaponAttachmentObj() { return WeaponAttachmentObj; }


	int32 getCurrentAmmo() { return CurrentAmmo; }
	int32 getCurrentMaxAmmo() { return CurrentMaxAmmo; }
	int32 getAmmoPerClip() { return AmmoPerClip; }
	int32 GetMaxAmmoCapacity() { return MaxAmmoCapacity; }


	FName GetWeaponName() { return WeaponName; }

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

	WeaponType GetWeaponType() {
		return weaponType;
	}
};
