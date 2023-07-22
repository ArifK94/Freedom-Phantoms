#include "Managers/GameHUDController.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

AGameHUDController::AGameHUDController()
{
}

void AGameHUDController::BeginPlay()
{
	Super::BeginPlay();

	/** Uncomment when debugging again. */
//	AddGameStatsViewPort();
}

UUserWidget* AGameHUDController::AddWidgetToViewport(TSubclassOf<UUserWidget> WidgetClass, bool AddToArray)
{
	if (WidgetClass)
	{
		UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);

		if (Widget)
		{
			UUserWidget* ViewportWidget;
			if (IsWidgetInViewport(Widget, ViewportWidget)) {
				return ViewportWidget;
			}

			Widget->AddToViewport();

			if (AddToArray)
			{
				ViewportWidgets.Add(Widget);
			}
		}

		return Widget;
	}

	return nullptr;
}

void AGameHUDController::ShowAllWidgets()
{
	for (auto Widget : ViewportWidgets)
	{
		if (Widget != InventoryWidget)
		{
			Widget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void AGameHUDController::HideAllWidgets()
{
	for (auto Widget : ViewportWidgets)
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AGameHUDController::RemoveAllWidgets()
{
	for (auto Widget : ViewportWidgets)
	{
		Widget->RemoveFromParent();
	}

	ViewportWidgets.Empty();
}

bool AGameHUDController::IsWidgetInViewport(UUserWidget* Widget, UUserWidget*& ViewportWidget)
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UUserWidget::StaticClass(), true);

	for (auto W : FoundWidgets)
	{
		if (W->GetClass()->GetName() == Widget->GetClass()->GetName())
		{
			ViewportWidget = Widget;
			return true;
		}
	}

	return false;
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

	if (InventoryWidget) {
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AGameHUDController::AddEndGameWidget()
{
	EndGameWidget = AddWidgetToViewport(EndGameWidgetClass);
}
