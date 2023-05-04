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

		return Widget;
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

void AGameHUDController::AddPlayerWidgets()
{
	CommanderHUDWidget = AddWidgetToViewport(CommanderHUDWidgetClass);
	WeaponAmmoWidget = AddWidgetToViewport(WeaponAmmoWidgetClass);
	WeaponCrosshairWidget = AddWidgetToViewport(WeaponCrosshairWidgetClass);
	HealthWidget = AddWidgetToViewport(HealthWidgetClass);
	InteractWidget = AddWidgetToViewport(InteractWidgetClass);
	SupportPackageWidget = AddWidgetToViewport(SupportPackageWidgetClass);
	ObjectiveWidget = AddWidgetToViewport(ObjectiveWidgetClass);

	InventoryWidget = AddWidgetToViewport(InventoryWidgetClass);
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AGameHUDController::AddEndGameWidget()
{
	EndGameWidget = AddWidgetToViewport(EndGameWidgetClass);
}
