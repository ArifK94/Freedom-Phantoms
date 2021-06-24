#include "Managers/GameInstanceController.h"
#include "Weapons/Weapon.h"
#include "Props/SupportPackage.h"

#include "MoviePlayer.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"


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

ACombatCharacter* UGameInstanceController::SpawnCombatCharacter()
{
	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;


	AActor* PlayerStartActor = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
	if (PlayerStartActor)
	{
		TargetLocation = PlayerStartActor->GetActorLocation();
		TargetRotation = PlayerStartActor->GetActorRotation();
	}

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
	return GetWorld()->SpawnActor<AWeapon>(PrimaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
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