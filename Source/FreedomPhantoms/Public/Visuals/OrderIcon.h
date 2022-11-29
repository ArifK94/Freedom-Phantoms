#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "EnumCollection.h"
#include "OrderIcon.generated.h"

class UWidgetComponent;
UCLASS()
class FREEDOMPHANTOMS_API AOrderIcon : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EIconType IconType;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USceneComponent* Root;

	/** The root & its children components will be animated */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USceneComponent* AnimationRoot;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	//	UWidgetComponent* WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> AttackWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> DefendWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> FollowWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UUserWidget> WoundedWidgetClass;




	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AActor> AttackIconClass;
	AActor* AttackIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AActor> DefendIconClass;
	AActor* DefendIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AActor> FollowIconClass;
	AActor* FollowIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AActor> WoundedIconClass;
	AActor* WoundedIcon;

	TArray<AActor*> Icons;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* CurveFloat;
	FTimeline CurveTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float TargetPosAmountZ;


	/** Anything less or equal to 0.0f will be considered as unlimited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DisplayCountDown;


private:
	FTimerHandle THandler_Countdown;

	FVector OrginalPos;
	FVector TargetPos;

public:	
	AOrderIcon();

	void ShowIcon(EIconType SelectedIconType, bool CountdownHideIcon = true);
	void ShowIcon(FVector Location, bool CountdownHideIcon = true);
	void ShowIcon(bool CountdownHideIcon = true);
	void HideIcon();

private:
	UFUNCTION()
		void BeginAnimation(float Value);

	void FacePlayer();
	AActor* SpawnIcon(TSubclassOf<AActor> IconClass);
	void DisplayIcon(AActor* SelectedIcon);


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
