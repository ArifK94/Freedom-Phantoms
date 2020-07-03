// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Engine/DataTable.h"


#include "Weapon.generated.h"



class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class USoundBase;
class UAudioComponent;
class UArrowComponent;
class UAnimMontage;
class UBlendSpace;
class UBlendSpace1D;
class UAnimationAsset;
class UAnimMontage;
class UAnimSequence;
class UAimOffsetBlendSpace;

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class WeaponType : uint8
{
	Rifle		UMETA(DisplayName = "Rifle"),
	SMG 		UMETA(DisplayName = "SMG"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	LMG			UMETA(DisplayName = "LMG"),
	Pistol		UMETA(DisplayName = "Pistol")
};

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class SelectiveFire : uint8
{
	Automatic		UMETA(DisplayName = "Automatic"),
	SemiAutomatic 	UMETA(DisplayName = "SemiAutomatic"),
	Burst			UMETA(DisplayName = "Burst"),
	Single			UMETA(DisplayName = "Single")
};

USTRUCT(BlueprintType)
struct FWeaponAnimSet : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		WeaponType weaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimSequence* ArmedSprintStart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimSequence* ArmedSprintLoop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimSequence* ArmedSprintStop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace1D* StandArmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace1D* CrouchArmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* StandAimingBS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBlendSpace* CrouchAimingBS;

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



UCLASS()
class FREEDOMFIGHTERS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

protected:
	virtual void Fire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void OnReload();

	void ConfigSetup();

	void SpawnMagazine();

	void Recoil();

	virtual FVector getMuzzleLocation();


public:

	void StartFire();

	void StopFire();

	void BeginFireEffect(FVector TraceEnd);

	void BeginShellEffect();

	void CameraShakeEffect();


	void BeginReload();

	void EndReload();

	void ClipIn();

	void ClipOut();

	void SetMagazineSocket();

	void SetClipSocket(USkeletalMeshComponent* meshComponent);

	int32 getCurrentAmmo();
	int32 getMaxAmmo();
	int32 getAmmoPerClip();

	void setWeaponSocket(USkeletalMeshComponent* meshComponent, FName socket);

	FName getHolsterSocket();
	FName getWeaponHandSocket();

	void SpawnWeaponAttachments();


	void SetHandGuardIK(USkeletalMeshComponent* CharacterMesh);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		FVector MuzzleLocationTest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		FRotator MuzzleRotationTest;

protected:

	float CurrentDeltaTime;

	class UGameInstanceController* gameInstanceController;

	USkeletalMeshComponent* CharacterReference;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Hand Offsets", meta = (AllowPrivateAccess = "true"))
		USceneComponent* HandguardMesh;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Ammo", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class AWeaponClip> weaponClip;
	class AWeaponClip* weaponClipObj;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		SelectiveFire selectiveFireMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Attachment", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UWeaponAttachmentManager> WeaponAttachmentClass;
	UWeaponAttachmentManager* WeaponAttachmentObj;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Ammo")
		bool canShowClip;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		bool canAutoReload;

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
		FName WeaponHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName EjectorSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName HandguardSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sockets", meta = (AllowPrivateAccess = "true"))
		FName ReloadClipHandSocket;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Hand Offsets", meta = (AllowPrivateAccess = "true"))
		FTransform HandguardOffset;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float RateOfFire;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Ammo")
		int32 CurrentAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Ammo")
		int32 MaxAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Ammo")
		int32 AmmoPerClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Ammo")
		FString AmmoCount;

	FTimerHandle THandler_TimeBetweenShots;

	// Derived from RateOfFire
	float TimeBetweenShots;

	float LastFireTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Recoil")
		float RecoilAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Recoil")
		float HorizontalRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Recoil")
		float VerticleRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Recoil", meta = (AllowPrivateAccess = "true"))
		float TargetVerticalRecoil;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Recoil", meta = (AllowPrivateAccess = "true"))
		float	TargetHorizontalRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ShotSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ReloadClipOutSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ReloadClipInSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Sounds")
		USoundBase* ReloadEndSound;

	UAudioComponent* ShotAudioComponent;
	UAudioComponent* ClipAudioComponent;




	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Particle Effects", meta = (AllowPrivateAccess = "true"))
		UParticleSystem* ShellEjectEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Damage")
		float BaseDamage;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool isFiring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		bool isReloading;


	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;


	void setCharacter(USkeletalMeshComponent* mesh) { CharacterReference = mesh; }
	USkeletalMeshComponent* getCharacter() { return CharacterReference; }

	USkeletalMeshComponent* getMeshComp() {
		return MeshComp;
	}


	UWeaponAttachmentManager* getWeaponAttachmentObj() {
		return WeaponAttachmentObj;
	}
};
