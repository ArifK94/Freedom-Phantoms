#include "Managers/GameInstanceController.h"

#include "MoviePlayer.h"
#include "Components/Widget.h"

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