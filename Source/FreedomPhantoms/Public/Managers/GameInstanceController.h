#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "StructCollection.h"
#include "GameInstanceController.generated.h"

class AWeapon;
class ACombatCharacter;
class ASupportPackage;
class USoundClass;
class USoundMix;
class UDatatableManager;

UCLASS()
class FREEDOMPHANTOMS_API UGameInstanceController : public UGameInstance
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FMapDetail SelectedLevel;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
	UUserWidget* LoadingScreenWidget;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> PrimaryWeaponClass;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWeapon> SecondaryWeaponClass;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<ASupportPackage>> SupportPackageClasses;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<ACombatCharacter> CombatCharacterClass;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UDatatableManager> DatatableManagerClass;

	UPROPERTY()
		UDatatableManager* DatatableManager;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsBloodEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		bool IsCrosshairEnabled = true;

	/** For UMG main menu to allow other widgets to be translated downwards to accomodate the navbar height and display their UI elements below it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		FVector2D MenuNavbarSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saved Slots", meta = (AllowPrivateAccess = "true"))
		FString FirstTimeSaveSlotName;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		FString AudioSettingsSaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundMix* SoundMixModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundClass* MasterSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundClass* SFXSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundClass* VoiceSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
		USoundClass* MusicSoundClass;

public:
	virtual void Init() override;

	/** Any functions which need time to load up goes here */
	UFUNCTION(BlueprintCallable)
		void DelayedInit();

	UFUNCTION()
		virtual void BeginLoadingScreen(const FString& MapName);
	UFUNCTION()
		virtual void EndLoadingScreen(UWorld* InLoadedWorld);

	UFUNCTION(BlueprintCallable)
		void SetPrimaryWeaponClass(TSubclassOf<AWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable)
		void SetSecondaryWeaponClass(TSubclassOf<AWeapon> WeaponClass);

	UFUNCTION(BlueprintCallable)
		void SetCombatCharacterClass(TSubclassOf<ACombatCharacter> CharacterClass);

	UFUNCTION(BlueprintCallable)
		void ToggleBlood();

	UFUNCTION(BlueprintCallable)
		void ToggleCrosshairs();


	AWeapon* SpawnPrimaryWeapon(AActor* Owner);

	AWeapon* SpawnSecondaryWeapon(AActor* Owner);

	ACombatCharacter* SpawnCombatCharacter(FVector TargetLocation, FRotator TargetRotation);

	TArray<ASupportPackage*> GetSupportPackage();

private:
	void LoadAudioSettings();

public:

	UDatatableManager* GetDatatableManager() {
		return DatatableManager;
	}

	bool GetIsBloodEnabled() {
		return IsBloodEnabled;
	}

	bool GetIsCrosshairEnabled() {
		return IsCrosshairEnabled;
	}

};
