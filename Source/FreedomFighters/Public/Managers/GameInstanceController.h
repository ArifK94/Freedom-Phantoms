#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInstanceController.generated.h"

class AWeapon;
class ACombatCharacter;
class ASupportPackage;
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


    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
        TArray<TSubclassOf<ASupportPackage>> SupportPackageClasses;

    TSubclassOf<ACombatCharacter> CombatCharacterClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        bool IsBloodEnabled = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        bool IsCrosshairEnabled = true;


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

    UFUNCTION(BlueprintCallable)
        void ToggleBlood();

    UFUNCTION(BlueprintCallable)
        void ToggleCrosshairs();


    AWeapon* SpawnPrimaryWeapon(AActor* Owner);

    AWeapon* SpawnSecondaryWeapon(AActor* Owner);

    ACombatCharacter* SpawnCombatCharacter();

    TArray<ASupportPackage*> GetSupportPackage();



    bool GetIsBloodEnabled() {
        return IsBloodEnabled;
    }

    bool GetIsCrosshairEnabled() {
        return IsCrosshairEnabled;
    }

};
