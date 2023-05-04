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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> CommanderHUDWidgetClass;
	UUserWidget* CommanderHUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> HealthWidgetClass;
	UUserWidget* HealthWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WeaponAmmoWidgetClass;
	UUserWidget* WeaponAmmoWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WeaponCrosshairWidgetClass;
	UUserWidget* WeaponCrosshairWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> InventoryWidgetClass;
	UUserWidget* InventoryWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> SupportPackageWidgetClass;
	UUserWidget* SupportPackageWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> InteractWidgetClass;
	UUserWidget* InteractWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> ObjectiveWidgetClass;
	UUserWidget* ObjectiveWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> EndGameWidgetClass;
	UUserWidget* EndGameWidget;


public:
	AGameHUDController();

	UUserWidget* AddWidgetToViewport(TSubclassOf<UUserWidget> WidgetClass);

	void CreateNVGWidget();
	
	void AddGameStatsViewPort();

	UFUNCTION(BlueprintCallable)
		void AddPlayerWidgets();

	void AddEndGameWidget();

private:
	virtual void BeginPlay() override;

public:
	UUserWidget* GetInventoryWidget() { return InventoryWidget; }
	UUserWidget* GetEndGameWidget() { return EndGameWidget; }

};
