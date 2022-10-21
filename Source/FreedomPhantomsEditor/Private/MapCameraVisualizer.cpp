// Fill out your copyright notice in the Description page of Project Settings.


#include "MapCameraVisualizer.h"
#include "Props/MapCamera.h"

#include "Components/SceneCaptureComponent2D.h"

void FMapCameraVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	// Get our move component
	const auto SceneCaptureComponent = Cast<USceneCaptureComponent2D>(Component);
	if (SceneCaptureComponent && SceneCaptureComponent->GetOwner())
	{
		auto MapCamera = Cast<AMapCamera>(SceneCaptureComponent->GetOwner());

		if (MapCamera)
		{
			PDI->DrawLine(
				MapCamera->GetActorLocation(),
				MapCamera->GetActorLocation() + FVector(MapCamera->GetPanningMin().X, 0.f, 0.f),
				FLinearColor::Red,
				SDPG_Foreground
			);

			PDI->DrawLine(
				MapCamera->GetActorLocation(),
				MapCamera->GetActorLocation() + FVector(MapCamera->GetPanningMax().X, 0.f, 0.f),
				FLinearColor::Red,
				SDPG_Foreground
			);

			PDI->DrawLine(
				MapCamera->GetActorLocation(),
				MapCamera->GetActorLocation() + FVector(0.f, MapCamera->GetPanningMin().Y, 0.f),
				FLinearColor::Green,
				SDPG_Foreground
			);

			PDI->DrawLine(
				MapCamera->GetActorLocation(),
				MapCamera->GetActorLocation() + FVector(0.f, MapCamera->GetPanningMax().Y, 0.f),
				FLinearColor::Green,
				SDPG_Foreground
			);


			PDI->DrawLine(
				MapCamera->GetActorLocation(),
				MapCamera->GetActorLocation() + FVector(0.f, 0.f, -MapCamera->GetZoomMax()),
				FLinearColor::Yellow,
				SDPG_Foreground
			);

			PDI->DrawLine(
				MapCamera->GetActorLocation(),
				MapCamera->GetActorLocation() + FVector(0.f, 0.f, MapCamera->GetZoomMin()),
				FLinearColor::Yellow,
				SDPG_Foreground
			);
		}


	}
}
