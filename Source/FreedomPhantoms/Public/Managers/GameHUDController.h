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
	TArray<UUserWidget*> ViewportWidgets;

	UUserWidget* NVGWidget;
	UUserWidget* GameStatWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> CommanderHUDWidgetClass;
	UUserWidget* CommanderHUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> HealthWidgetClass;
	UUserWidget* HealthWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WeaponAmmoWidgetClass;
	UUserWidget* WeaponAmmoWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WeaponCrosshairWidgetClass;
	UUserWidget* WeaponCrosshairWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> InventoryWidgetClass;
	UUserWidget* InventoryWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> SupportPackageWidgetClass;
	UUserWidget* SupportPackageWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> InteractWidgetClass;
	UUserWidget* InteractWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> ObjectiveWidgetClass;
	UUserWidget* ObjectiveWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Widgets", meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> EndGameWidgetClass;
	UUserWidget* EndGameWidget;


public:
	AGameHUDController();

	/** Add widget to the array of widgets which stores a collection of all widgets added to viewport in order to perform CRUD operations? */
	UFUNCTION(BlueprintCallable)
		UUserWidget* AddWidgetToViewport(TSubclassOf<UUserWidget> WidgetClass, bool AddToArray = true);

	/** Display all widgets except for those that are collapsed by default such as inventory widget. */
	UFUNCTION(BlueprintCallable)
		void ShowAllWidgets();

	/** Collapse all widgets. */
	UFUNCTION(BlueprintCallable)
		void HideAllWidgets();

	/** Remove all widgets from the collection. */
	UFUNCTION(BlueprintCallable)
		void RemoveAllWidgets();

	bool IsWidgetInViewport(UUserWidget* Widget, UUserWidget*& ViewportWidget);

	void CreateNVGWidget();
	
	UFUNCTION(BlueprintCallable)
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
