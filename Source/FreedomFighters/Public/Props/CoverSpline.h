// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"

#include "Engine/DataTable.h"


#include "CoverSpline.generated.h"


UENUM(BlueprintType)
enum class ESplineMeshType : uint8 {
	DEFAULT		UMETA(DisplayName = "Default Mesh"),
	START		UMETA(DisplayName = "Starting Mesh"),
	END			UMETA(DisplayName = "EndingMesh"),
};

USTRUCT(BlueprintType)
struct FSplineMeshDetails : public FTableRowBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		class UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		class UMaterialInterface* AlternativeMaterial;

	FSplineMeshDetails() : ForwardAxis(ESplineMeshAxis::Type::X)
	{
	}
};


UCLASS()
class FREEDOMFIGHTERS_API ACoverSpline : public AActor
{
	GENERATED_BODY()

public:
	ACoverSpline();

	void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void FollowSpline(APawn* Character, float InputVal);

	int GetTotalSplinePoints();

	USplineComponent* GetSplineComponent() {
		return SplineComponent;
	}

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spline")
		TMap<ESplineMeshType, FSplineMeshDetails> SplineMeshMap;

	UPROPERTY(VisibleAnywhere, Category = "Spline")
		USplineComponent* SplineComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spline")
		FRotator SplineRotation;


	UFUNCTION()
		void OnCoverBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	/* Sets the rotation on a spline point Local rotation only.*/
	UFUNCTION(BlueprintCallable, Category = "Splines") //make the function usable in blueprints. BlueprintPure doesn't require exec; BlueprintCallable does.
		void set_rotation_at_spline_point(USplineComponent* target, const int32 point_index, const FRotator rotation); //define inputs and outputs. Const for inputs, and & for outputs.

};
