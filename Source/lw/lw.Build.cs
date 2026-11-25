// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class lw : ModuleRules
{
	public lw(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"SlateCore",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "NavigationSystem" });

		PublicIncludePaths.AddRange(new string[] {
			"lw",
			"lw/Variant_Platforming",
			"lw/Variant_Platforming/Animation",
			"lw/Variant_Combat",
			"lw/Variant_Combat/AI",
			"lw/Variant_Combat/Animation",
			"lw/Variant_Combat/Gameplay",
			"lw/Variant_Combat/Interfaces",
			"lw/Variant_Combat/UI",
			"lw/Variant_SideScrolling",
			"lw/Variant_SideScrolling/AI",
			"lw/Variant_SideScrolling/Gameplay",
			"lw/Variant_SideScrolling/Interfaces",
			"lw/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
