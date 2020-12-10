// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RenderStream : ModuleRules
{
	public RenderStream(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Slate", "Networking", "Sockets" });

		PrivateDependencyModuleNames.AddRange(new string[] { "MovieSceneCapture" });

		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
