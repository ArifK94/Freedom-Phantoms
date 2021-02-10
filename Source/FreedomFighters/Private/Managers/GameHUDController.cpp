#include "Managers/GameHUDController.h"

#include "Blueprint/WidgetLayoutLibrary.h"

AGameHUDController::AGameHUDController()
{

}


void AGameHUDController::BeginPlay()
{
	Super::BeginPlay();

	CreateAC130Widget();
}

void AGameHUDController::CreateNVGWidget()
{
	if (NVGWidgetClass)
	{
		NVGWidget = CreateWidget<UUserWidget>(GetWorld(), NVGWidgetClass);
		if (NVGWidget)
		{
			NVGWidget->AddToViewport();
		}
	}
}

void AGameHUDController::CreateAC130Widget()
{
	if (AC130WidgetClass)
	{
		AC130Widget = CreateWidget<UUserWidget>(GetWorld(), AC130WidgetClass);
	}
}

void AGameHUDController::AddAC130ViewPort()
{
	CreateAC130Widget();

	if (AC130Widget)
	{
		UWidgetLayoutLibrary::RemoveAllWidgets(GetWorld());
		AC130Widget->AddToViewport();
	}
}
void AGameHUDController::RemoveAC130ViewPort()
{
	if (AC130Widget)
	{
		AC130Widget->RemoveFromViewport();
	}
}
