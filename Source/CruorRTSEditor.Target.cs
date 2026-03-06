// Copyright Cruor ehf., All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CruorRTSEditorTarget : TargetRules
{
	public CruorRTSEditorTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.V6;

		Type = TargetType.Editor;
		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor", "Cruor" });

		if (!bBuildAllModules)
		{
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}

		LyraGameTarget.ApplySharedLyraTargetSettings(this);

		EnablePlugins.Add("RemoteSession");
	}
}