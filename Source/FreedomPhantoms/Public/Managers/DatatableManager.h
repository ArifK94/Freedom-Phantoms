// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StructCollection.h"
#include "DatatableManager.generated.h"

/**
 * Store datatables in this object to be retrieved from multiple classes.
 */
UCLASS(Blueprintable, BlueprintType)
class FREEDOMPHANTOMS_API UDatatableManager : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UDataTable* SurfaceImpactSetDatatable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UDataTable* SurfaceImpactDatatable;

public:
	virtual UWorld* GetWorld() const override;

	static FSurfaceImpactSet* RetrieveSurfaceImpactSet(UWorld* WorldContextObject, FName RowName);
	UDataTable* GetSurfaceImpactSetDatatable() { return SurfaceImpactSetDatatable; }

	static FSurfaceImpact* RetrieveSurfaceImpact(UWorld* WorldContextObject, FName RowName);
	UDataTable* GetSurfaceImpactDatatable() { return SurfaceImpactDatatable; }

	
private:
	static UDatatableManager* GetDatatableManagerInstance(UWorld* WorldContextObject);
};
