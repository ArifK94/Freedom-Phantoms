#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumCollection.h"
#include "StructCollection.h"
#include "Interfaces/Interactable.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UNiagaraSystem;
class USoundBase;
class USoundCue;
class UAudioComponent;
class AWeaponAttachment;
class AWeaponClip;
class AProjectile;
class UObjectPoolComponent;
class UTexture;
class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponUpdateSignature, FWeaponUpdateParameters, WeaponUpdateParameters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmptyAmmoClipSignature, AWeapon*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKillConfirmedSignature, FProjectileImpactParameters, ProjectileImpactParameters);

UCLASS()
class FREEDOMPHANTOMS_API AWeapon : public AActor, public IInteractable
{
	GENERATED_BODY()

private:
	FTimerHandle THandler_DelayedInit;
	FTimerHandle THandler_BulletSpread;
	FTimerHandle THandler_BurstFire;
	int BurstAmmountCount;

	/** Destroy the actor */
	UFUNCTION()
		void OnDestroyWeapon(AActor* Actor);

	UPROPERTY(Transient)
		TObjectPtr<class UGameInstance>	OwningGameInstance;

protected:

	UPROPERTY()
		USkeletalMeshComponent* CharacterReference;


	// Derived from RateOfFire
	float TimeBetweenShots;
	float LastFireTime;
	FTimerHandle THandler_TimeBetweenShots;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* MeshComp;

	/** The Parent mesh by default is the MeshComp, but if chosen to use attached parent actor mesh such as tank cannon, then this would use the tank's mesh cannon's sockets to shoot from */
	USceneComponent* ParentMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* HandguardMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UObjectPoolComponent* ObjectPoolComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UPointLightComponent* MuzzleLightComponent;
	FTimerHandle THandler_MuzzleLight;


	//----------------- ATTACHMENTS -----------------//


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments", meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AWeaponAttachment>> ScopeAttachmentClasses;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachments", meta = (AllowPrivateAccess = "true"))
		AWeaponAttachment* ScopeAttachment;



	/** Not all weapons may need to have bullet object pooled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool UseObjectPool;

	/** Some imported models such as tanks or helicopters have turrets built in but cannot be separated actors, so using the vehicle's turret muzzles should be used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool UseParentMuzzle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeaponClip> weaponClip;

	UPROPERTY()
		AWeaponClip* weaponClipObj;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AProjectile> BulletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		SelectiveFire selectiveFireMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
		bool canShowClip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool isFiring;

	/** Prevent shooting montage from stopping if pressed to fire but has not fired a shot, making sure that the weapon has been shot before stopping shooting montage */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool HasFiredFirstShot;

	/** 
	* If user has decided to stop but the first shot has not been fired, then the weapon will constantly fire if it is an automatic weapon.
	* This flag will check after the first shot has been fired.
	*/
	bool ShouldStopFiring;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool isReloading;
	bool IsAiming;


	/** Useful for cannons, as they may not have reload animation but instead a countdown reload time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanAutoReload;
	FTimerHandle THandler_AutoReloadBegin;
	FTimerHandle THandler_AutoReloadEnd;

	/** For image to be displayed when selecting a weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTexture* WeaponThumbnail;

	/** For image to be displayed next to current weapon during gameplay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UTexture* WeaponThumbnailWireframe;

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

	/** How far out should the weapon be allowed to fire, uses dot product ot check if weapon is facing in owner's eye forward direction. Zero means there will be no tolerance  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", ClampMin = 0.0f, ClampMax = 1.0f))
		float CrosshairErrorTolerance;


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

	/** Prevent the player from scavenging this weapon for ammo again */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
		bool IsScavenged;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
		float ZoomFOV;


	/** RECOIL */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (AllowPrivateAccess = "true"))
		bool hasRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
		float BulletSpreadMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.0f))
		float BulletSpreadMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.1f))
		float BulletSpreadReduceRate;

	/** More better results for shotguns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (AllowPrivateAccess = "true", ClampMin = 0.1f))
		bool UseRadialSpread;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Recoil")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
		USoundBase* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* FireAttenuation;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ShotAudioComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		bool PlayFireSoundAtLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sounds", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ClipAudioComponent;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ShellEjectEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* MuzzleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Effects", meta = (AllowPrivateAccess = "true"))
		UNiagaraSystem* MuzzleFlashNiagara;

	/** Message to be displayed on the UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
		FName PickupMessage;


	/** Charging Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charging", meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ChargingAudioComponent;

	/** Starting from the low charge sound down to high charge sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charging", meta = (AllowPrivateAccess = "true"))
		TArray<FCrossfadeAudio> ChargeUpSounds;

	/** Starting from the high charge sound down to low charge sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charging", meta = (AllowPrivateAccess = "true"))
		TArray<FCrossfadeAudio> ChargeDownSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charging", meta = (AllowPrivateAccess = "true"))
		float ChargeUpTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charging", meta = (AllowPrivateAccess = "true"))
		bool IsChargingUp;

	float CurrentChargeAmount;
	FTimerHandle THandler_ChargeUp;
	FTimerHandle THandler_ChargeDown;


	/** Can this weapon be used for locking onto target actors? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Actor", meta = (AllowPrivateAccess = "true"))
		bool CanLockActors;

	UPROPERTY()
		AActor* TargetActor;


	/** Debug */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		bool DrawDebugShotLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
		float ShotLineDuration;


private:

	/** Used for vehicles rather than pawns */
	UPROPERTY()
		USceneComponent* EyeViewPointComponent;


public:
	AWeapon();

	UPROPERTY(BlueprintAssignable)
		FOnWeaponUpdateSignature OnWeaponUpdate;

	UPROPERTY(BlueprintAssignable)
		FOnEmptyAmmoClipSignature OnEmptyAmmoClip;

	UPROPERTY(BlueprintAssignable)
		FOnKillConfirmedSignature OnKillConfirmed;

	UFUNCTION()
		void OnProjectileImpacted(FProjectileImpactParameters ProjectileImpactParameters);

	// Interactable interface methods
	virtual FString GetKeyDisplayName_Implementation() override;
	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool OnUseInteraction_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool CanInteract_Implementation(APawn* InPawn, AController* InController) override;

	virtual void Fire();

	UFUNCTION(BlueprintCallable)
		virtual void StartFire();

	UFUNCTION(BlueprintCallable)
		void StopFire();

	/** Reset to Default. Useful for swapping weapons then making sure weapon can be fired again. */
	void ReadyToUse();

	bool CanFireWeapon();

	void BeginReload();

	void EndReload();

	void ClipIn();

	void ClipOut();

	void SetMagazineSocket();

	void SetClipSocket(USkeletalMeshComponent* meshComponent);

	virtual void setWeaponSocket(USkeletalMeshComponent* meshComponent, FName socket);

	void SetHandGuardIK(USkeletalMeshComponent* CharacterMesh, FName TriggerHandSocket);

	/** Return bool so it can be used to play sounds if true */
	bool ReplenishAmmo(int Amount = -1);

	virtual void SetIsAiming(bool isAiming);

	void SetWeaponProfile(FName InCollisionProfileName);

	virtual void HolsterWeapon(USkeletalMeshComponent* Parent);

	void ToggleVisibility(bool Enabled);

	/** Called when owning character has died or picking up another weapon */
	virtual void DropWeapon(bool RemoveOwner = true, bool SimulatePhysics = false);

private:
	void LoadParentMesh();

	void ConvertWeaponName();

	void AutoReloadBegin();
	void AutoReloadEnd();

	// Charging the weapon before being able to fire or used when holding the aim button eg. minigun aiming
	void ChargeUp();

	void ChargeDown();

	void IncreaseCharge();

	void DecreaseCharge();

	FVector BulletSpread(FVector Spread);

	void ReduceBulletSpread();

	FVector BulletSpreadRadial(float Radius);

	FRotator GetSprayAngle(FVector MuzzleDirection, float MaxAngle);

	void DisableMuzzleLight();

protected:
	virtual void BeginPlay() override;

	void DelayedInit();


	bool IsFacingCrosshair();


	virtual void CreateBullet();

	void SpawnProjectile(FVector Locatiom, FRotator Rotation);

	UFUNCTION(BlueprintCallable)
		virtual void OnReload();

	void ConfigSetup();

	void SpawnScopeAttachment();

	void SpawnMagazine();

	virtual FVector getMuzzleLocation();

	FRotator GetMuzzleRotation();

	void BeginShellEffect();

	void PlayShotEffect(FVector EyeLocation);

	void EmptyClipEvent();

	virtual UWorld* GetMyWorld() const;

	class FTimerManager& GetTimerManager() const;

public:

	void setCharacter(USkeletalMeshComponent* mesh) { CharacterReference = mesh; }
	USkeletalMeshComponent* getCharacter() { return CharacterReference; }

	USkeletalMeshComponent* GetMeshComp() { return MeshComp; }

	int32 GetCurrentAmmo() { return CurrentAmmo; }
	int32 getCurrentMaxAmmo() { return CurrentMaxAmmo; }
	int32 getAmmoPerClip() { return AmmoPerClip; }
	int32 GetMaxAmmoCapacity() { return MaxAmmoCapacity; }


	FName GetWeaponName() { return WeaponName; }

	FName GetMuzzleSocket() { return MuzzleSocket; }
	FName getParentHolderSocket() { return ParentHolderSocket; }
	FName getOpticsSocket() { return OpticsSocket; }
	FName getLaserSocket() { return LaserSocket; }
	FName getTorchlightSocket() { return TorchlightSocket; }

	FName GetPickupMessage() { return PickupMessage; }

	bool getIsReloading() { return isReloading; }
	bool getIsFiring() { return isFiring; }
	bool GetHasFiredFirstShot() { return HasFiredFirstShot; }


	bool GetHasUnlimitedAmmo() { return HasUnlimitedAmmo; }
	void SetUnlimitedAmmo(bool IsUnlimited) { HasUnlimitedAmmo = IsUnlimited; }

	USceneComponent* GetEyeViewPointComponent() { return EyeViewPointComponent; }
	void SetComponentEyeViewPoint(USceneComponent* Comp) { EyeViewPointComponent = Comp; }

	void SetCrosshairErrorTolerance(float Value) { CrosshairErrorTolerance = Value; }

	float GetZoomFOV() { return ZoomFOV; }

	UObjectPoolComponent* GetObjectPoolComponent() { return ObjectPoolComponent; }

	UAudioComponent* GetShotAudioComponent() { return ShotAudioComponent; }

	UAudioComponent* GetClipAudioComponent() { return ClipAudioComponent; }

	WeaponType GetWeaponType() { return weaponType; }

	bool GetIsScavenged() { return IsScavenged; }
	void SetIsScavenged(bool Scavenged) { IsScavenged = Scavenged; }


	AActor* GetTargetActor() { return TargetActor; }
	void SetTargetActor(AActor* Target) { TargetActor = Target; }

};
