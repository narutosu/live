// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class live : ModuleRules
{
	public live(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"Slate",
			"SlateCore",
		});

		PublicIncludePaths.AddRange(new string[] {
			"live",
			"live/Variant_Strategy",
			"live/Variant_Strategy/UI",
			"live/Variant_TwinStick",
			"live/Variant_TwinStick/AI",
			"live/Variant_TwinStick/Gameplay",
			"live/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
