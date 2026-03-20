// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraEditorTarget : TargetRules
{
	public LyraEditorTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V6;

		Type = TargetType.Editor;
		/*
		 * Start @CruorRTS_CodeModification
		 * Adding our Custom Module to the EditorTarget
		 */
		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor", "CruorRTS" });
		/*
		 *  End @CruorRTS_CodeModification
		 */

		if (!bBuildAllModules)
		{
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}

		LyraGameTarget.ApplySharedLyraTargetSettings(this);

		// This is used for touch screen development along with the "Unreal Remote 2" app
		EnablePlugins.Add("RemoteSession");
	}
}
