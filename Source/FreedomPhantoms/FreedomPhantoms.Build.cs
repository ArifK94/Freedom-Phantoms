// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FreedomPhantoms : ModuleRules
{
	public FreedomPhantoms(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",  
			"HeadMountedDisplay",
            "GameplayTasks",
            "UMG",
            "AIModule",
            "NavigationSystem",
            "PhysicsCore",
            "MoviePlayer",
            "Niagara",
            "AnimGraphRuntime",
			"CableComponent"
		});
	}
}
