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
		class UBoxComponent* CoverArea;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* ForwardDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool IsCoverLow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cover", meta = (AllowPrivateAccess = "true"))
		bool IsCorner;

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

	bool getIsCorner() {
		return IsCorner;
	};

	bool getCanTakeCover() {
		return CanTakeCover;
	}

	CoverCornerType getCornerType() {
		return  CornerType;
	}

public:
	ABaseCoverProp();
};
