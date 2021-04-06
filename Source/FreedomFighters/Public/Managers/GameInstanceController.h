#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInstanceController.generated.h"

/**
 * 
 */
UCLASS()
class FREEDOMFIGHTERS_API UGameInstanceController : public UGameInstance
{
	GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
    UUserWidget* LoadingScreenWidget;

public:
    virtual void Init() override;
    UFUNCTION()
        virtual void BeginLoadingScreen(const FString& MapName);
    UFUNCTION()
        virtual void EndLoadingScreen(UWorld* InLoadedWorld);
	
};
