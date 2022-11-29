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

	UPROPERTY(EditDefaultsOnly, Category = "Vehicles")
		TSubclassOf<UUserWidget> AC130WidgetClass;
private:
	UUserWidget* NVGWidget;
	UUserWidget* AC130Widget;


public:
	AGameHUDController();

	void CreateNVGWidget();
	
	void AddAC130ViewPort();
	void RemoveAC130ViewPort();

private:
	virtual void BeginPlay() override;

	void CreateAC130Widget();


};
