// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CruorRTSCoreRuntime : ModuleRules
{
	public CruorRTSCoreRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"AIModule",
				"Core", 
				"CruorRTS",
				"CommonInput", 
				"CommonUI",
				"GameplayAbilities",
				"GameplayTags", 
				"GameplayTasks",
				// ReSharper disable once CSharpBuildCSInvalidModuleName
				"LyraGame",
				"ModularGameplay",
				"ModularGameplayActors",
				"SignificanceManager",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CommonGame",
				"CoreUObject",
				"Engine",
				"EnhancedInput",
				"Slate",
				"SlateCore", 
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
