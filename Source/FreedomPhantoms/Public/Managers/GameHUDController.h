#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "Components/WidgetComponent.h"


#include "GameHUDController.generated.h"

UCLASS()
class FREEDOMPHANTOMS_API AGameHUDController : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Interactive")
		TSubclassOf<UUserWidget> NVGWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Stats")
		TSubclassOf<UUserWidget> GameStatsWidgetClass;
private:
	UUserWidget* NVGWidget;
	UUserWidget* GameStatWidget;


public:
	AGameHUDController();

	UUserWidget* AddWidgetToViewport(TSubclassOf<UUserWidget> WidgetClass);

	void CreateNVGWidget();
	
	void AddGameStatsViewPort();

private:
	virtual void BeginPlay() override;

};
