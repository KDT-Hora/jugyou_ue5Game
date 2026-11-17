using UnrealBuildTool;

public class EscapeRoomEditorTarget : TargetRules
{
	public EscapeRoomEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("EscapeRoom");
	}
}
