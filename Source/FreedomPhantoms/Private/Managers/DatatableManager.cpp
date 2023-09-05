// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/DatatableManager.h"
#include "Managers/GameInstanceController.h"

#include "Kismet/GameplayStatics.h"

UWorld* UDatatableManager::GetWorld() const
{
	UObject* Outer = GetOuter();
	
	if (!Outer) {
		return nullptr;
	}

	UGameInstanceController* GameInstanceController = Cast<UGameInstanceController>(GetOuter());

	if (!GameInstanceController) {
		return nullptr;
	}

	return GameInstanceController->GetWorld();
}


UDatatableManager* UDatatableManager::GetDatatableManagerInstance(UWorld* WorldContextObject)
{
	UGameInstanceController* GameInstanceController = Cast<UGameInstanceController>(UGameplayStatics::GetGameInstance(WorldContextObject));
	return GameInstanceController->GetDatatableManager();
}

FSurfaceImpactSet* UDatatableManager::RetrieveSurfaceImpactSet(UWorld* WorldContextObject, FName RowName)
{
	static const FString ContextString(TEXT("Surface Impact Set DataSet"));
	return GetDatatableManagerInstance(WorldContextObject)->SurfaceImpactSetDatatable->FindRow<FSurfaceImpactSet>(RowName, ContextString, true);
}

FSurfaceImpact* UDatatableManager::RetrieveSurfaceImpact(UWorld* WorldContextObject, FName RowName)
{
	static const FString ContextString(TEXT("Surface Impact DataSet"));
	return GetDatatableManagerInstance(WorldContextObject)->SurfaceImpactDatatable->FindRow<FSurfaceImpact>(RowName, ContextString, true);
}