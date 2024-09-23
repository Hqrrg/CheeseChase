// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CheeseChase : ModuleRules
{
	public CheeseChase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
