#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInstanceController.generated.h"

class AWeapon;
class ACombatCharacter;
UCLASS()
class FREEDOMFIGHTERS_API UGameInstanceController : public UGameInstance
{
    GENERATED_BODY()

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
    UUserWidget* LoadingScreenWidget;

    TSubclassOf<AWeapon> PrimaryWeaponClass;
    TSubclassOf<AWeapon> SecondaryWeaponClass;


    TSubclassOf<ACombatCharacter> CombatCharacterClass;


public:
    virtual void Init() override;
    UFUNCTION()
        virtual void BeginLoadingScreen(const FString& MapName);
    UFUNCTION()
        virtual void EndLoadingScreen(UWorld* InLoadedWorld);

    UFUNCTION(BlueprintCallable)
        void SetPrimaryWeaponClass(TSubclassOf<AWeapon> WeaponClass);

    UFUNCTION(BlueprintCallable)
        void SetSecondaryWeaponClass(TSubclassOf<AWeapon> WeaponClass);

    UFUNCTION(BlueprintCallable)
        void SetCombatCharacterClass(TSubclassOf<ACombatCharacter> CharacterClass);

    AWeapon* SpawnPrimaryWeapon(AActor* Owner);

    AWeapon* SpawnSecondaryWeapon(AActor* Owner);

    ACombatCharacter* SpawnCombatCharacter();



};
