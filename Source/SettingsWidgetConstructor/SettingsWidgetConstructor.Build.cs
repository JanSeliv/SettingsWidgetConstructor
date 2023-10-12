﻿// Copyright (c) Yevhenii Selivanov.

using UnrealBuildTool;

public class SettingsWidgetConstructor : ModuleRules
{
	public SettingsWidgetConstructor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Latest;
		bEnableNonInlinedGenCppWarnings = true;
        
		PublicDependencyModuleNames.AddRange(new[]
			{
				"Core"
				, "UMG" // Created USettingsWidget
				, "GameplayTags" // Created FSettingTag
				, "DeveloperSettings" // Created USettingsDataAsset
			}
		);

		PrivateDependencyModuleNames.AddRange(new[]
			{
				"CoreUObject", "Engine", "Slate", "SlateCore" // Core
				, "DataRegistry" // Multiple Data Tables support
			}
		);

		if (Target.bBuildEditor)
		{
			// Include Editor modules that are used in this Runtime module
			PrivateDependencyModuleNames.AddRange(new[]
				{
					"UnrealEd" // FDataTableEditorUtils
				}
			);
		}
	}
}
