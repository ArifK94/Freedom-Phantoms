#include "Managers/GameInstanceController.h"
#include "Managers/CustomSaveGame.h"
#include "Managers/DatatableManager.h"
#include "Characters/CombatCharacter.h"
#include "Weapons/Weapon.h"
#include "Props/SupportPackage.h"

#include "MoviePlayer.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blueprint/UserWidget.h"


void UGameInstanceController::SetPrimaryWeaponClass(TSubclassOf<AWeapon> WeaponClass)
{
	PrimaryWeaponClass = WeaponClass;
}

void UGameInstanceController::SetSecondaryWeaponClass(TSubclassOf<AWeapon> WeaponClass)
{
	SecondaryWeaponClass = WeaponClass;
}

void UGameInstanceController::SetCombatCharacterClass(TSubclassOf<ACombatCharacter> CharacterClass)
{
	CombatCharacterClass = CharacterClass;
}

ACombatCharacter* UGameInstanceController::SpawnCombatCharacter(FVector TargetLocation, FRotator TargetRotation)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<ACombatCharacter>(CombatCharacterClass, TargetLocation, TargetRotation, SpawnParams);
}

AWeapon* UGameInstanceController::SpawnPrimaryWeapon(AActor* Owner)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<AWeapon>(PrimaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
}

AWeapon* UGameInstanceController::SpawnSecondaryWeapon(AActor* Owner)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<AWeapon>(SecondaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
}

TArray<ASupportPackage*> UGameInstanceController::GetSupportPackage()
{
	TArray<ASupportPackage*> Packages;

	for (TSubclassOf<ASupportPackage> SPClass : SupportPackageClasses)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASupportPackage* SupportPackage = GetWorld()->SpawnActor<ASupportPackage>(SPClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (SupportPackage)
		{
			SupportPackage->SetActorHiddenInGame(true);
			SupportPackage->SetHidden(true);

			SupportPackage->SetActorEnableCollision(false);
			SupportPackage->SetActorTickEnabled(false);
		}

		Packages.Add(SupportPackage);
	}

	return Packages;
}

void UGameInstanceController::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UGameInstanceController::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UGameInstanceController::EndLoadingScreen);

	DatatableManager = NewObject<UDatatableManager>(this, DatatableManagerClass);
}

void UGameInstanceController::DelayedInit()
{
	LoadAudioSettings();
}

void UGameInstanceController::BeginLoadingScreen(const FString& InMapName)
{
	if (IsRunningDedicatedServer()) {
		return;
	}

	if (LoadingScreenWidgetClass)
	{
		LoadingScreenWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingScreenWidgetClass);
	}


	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;

	if (LoadingScreenWidget)
	{
		LoadingScreen.WidgetLoadingScreen = LoadingScreenWidget->TakeWidget();
	}
	else
	{
		LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
	}


	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);

}

void UGameInstanceController::EndLoadingScreen(UWorld* InLoadedWorld)
{

}


void UGameInstanceController::ToggleBlood()
{
	IsBloodEnabled = !IsBloodEnabled;
}

void UGameInstanceController::ToggleCrosshairs() 
{
	IsCrosshairEnabled = !IsCrosshairEnabled;
}

void UGameInstanceController::LoadAudioSettings()
{
	if (SoundMixModifier == nullptr) {
		return;
	}
	
	UGameplayStatics::SetBaseSoundMix(GetWorld(), SoundMixModifier);

	bool DoesSaveExist = UGameplayStatics::DoesSaveGameExist(AudioSettingsSaveSlotName, 0);

	if (!DoesSaveExist) {
		return;
	}

	UCustomSaveGame* CustomSaveGame = Cast<UCustomSaveGame>(UGameplayStatics::LoadGameFromSlot(AudioSettingsSaveSlotName, 0));

	if (CustomSaveGame == nullptr) {
		return;
	}

	if (MasterSoundClass) {
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SoundMixModifier, MasterSoundClass, CustomSaveGame->GetMasterVolume());
	}

	if (SFXSoundClass) {
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SoundMixModifier, SFXSoundClass, CustomSaveGame->GetSFXVolume());
	}

	if (VoiceSoundClass) {
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SoundMixModifier, VoiceSoundClass, CustomSaveGame->GetVoiceVolume());
	}

	if (MusicSoundClass) {
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SoundMixModifier, MusicSoundClass, CustomSaveGame->GetMusicVolume());
	}

}
