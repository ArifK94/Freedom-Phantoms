// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FreedomFighters : ModuleRules
{
	public FreedomFighters(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "GameplayTasks", "AnimGraph", "AnimGraphRuntime", "NavigationSystem" });

        PrivateDependencyModuleNames.AddRange(new string[] {
              "UnrealED",
              "AnimGraph",
              "AnimGraphRuntime",
              "BlueprintGraph"});
    }
}
