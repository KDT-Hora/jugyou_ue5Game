using UnrealBuildTool;

public class EscapeRoomTarget : TargetRules
{
	public EscapeRoomTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("EscapeRoom");
	}
}
