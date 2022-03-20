#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumCollection.h"
#include "OrderIcon.generated.h"

class UWidgetComponent;
UCLASS()
class FREEDOMFIGHTERS_API AOrderIcon : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EIconType IconType;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UWidgetComponent* WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> AttackWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> DefendWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> FollowWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> HVTWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WoundedWidgetClass;


	/** Anything less or equal to 0.0f will be considered as unlimited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DisplayCountDown;


private:
	FTimerHandle THandler_Countdown;

public:	
	AOrderIcon();

	void ShowIcon(EIconType SelectedIconType, bool CountdownHideIcon = true);
	void ShowIcon(FVector Location);
	void ShowIcon(bool CountdownHideIcon = true);
	void HideIcon();

private:
	void RotateToPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
