#include "Managers/GameHUDController.h"

#include "Blueprint/WidgetLayoutLibrary.h"

AGameHUDController::AGameHUDController()
{

}

void AGameHUDController::BeginPlay()
{
	Super::BeginPlay();

	/** Uncomment when debugging again. */
//	AddGameStatsViewPort();
}

UUserWidget* AGameHUDController::AddWidgetToViewport(TSubclassOf<UUserWidget> WidgetClass)
{
	if (WidgetClass)
	{
		UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);

		if (Widget)
		{
			Widget->AddToViewport();
		}
	}

	return nullptr;
}

void AGameHUDController::CreateNVGWidget()
{
	NVGWidget = AddWidgetToViewport(NVGWidgetClass);
}

void AGameHUDController::AddGameStatsViewPort()
{
	GameStatWidget = AddWidgetToViewport(GameStatsWidgetClass);
}
