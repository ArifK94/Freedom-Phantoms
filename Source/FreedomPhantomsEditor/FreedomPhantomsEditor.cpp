#include "FreedomPhantomsEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "MapCameraVisualizer.h"

IMPLEMENT_GAME_MODULE(FFreedomPhantomsEditorModule, FreedomPhantomsEditor);

void FFreedomPhantomsEditorModule::StartupModule()
{
    // Check if editor is valid
     if (GUnrealEd)
     {
         // Registerin the move visualizer
         TSharedPtr<FMapCameraVisualizer> MoveVisualizer = MakeShareable(new FMapCameraVisualizer);
         if (MoveVisualizer.IsValid())
         {
             GUnrealEd->RegisterComponentVisualizer(USceneCaptureComponent2D::StaticClass()->GetFName(), MoveVisualizer);
             MoveVisualizer->OnRegister();
         }
     }
}

void FFreedomPhantomsEditorModule::ShutdownModule()
{
    // Check if editor is valid
     if (GUnrealEd)
     {
         GUnrealEd->UnregisterComponentVisualizer(USceneCaptureComponent2D::StaticClass()->GetFName());
     }
}