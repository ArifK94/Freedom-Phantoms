// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseCoverProp.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class CoverCornerType : uint8
{
	None		UMETA(DisplayName = "None"),
	Left		UMETA(DisplayName = "Left"),
	Right 		UMETA(DisplayName = "Right")
};



UCLASS()
class FREEDOMFIGHTERS_API ABaseCoverProp : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* CoverArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* ForwardDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool IsCoverLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool IsCoverHigh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool CanPeakLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool CanPeakRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool CanPeakUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool CanPeakDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		FVector PeakDirection;

		bool CanTakeCover;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		CoverCornerType CornerType;


public:

	UArrowComponent* getArrowDirection() {
		return ForwardDirection;
	}

	bool getIsCoverLow() {
		return IsCoverLow;
	}

	bool getCanTakeCover() {
		return CanTakeCover;
	}

	bool getCanPeakLeft() {
		return CanPeakLeft;
	}

	bool getCanPeakRight() {
		return CanPeakRight;
	}

	bool getCanPeakUp() {
		return CanPeakUp;
	}

	bool getCanPeakDown() {
		return CanPeakDown;
	}

	FVector getPeakDirection()
	{
		return PeakDirection;
	}

	CoverCornerType getCornerType() {
		return  CornerType;
	}

public:
	ABaseCoverProp();
};
