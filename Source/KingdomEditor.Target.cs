using UnrealBuildTool;

public class KingdomEditorTarget : TargetRules
{
	public KingdomEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("KingdomGame");
	}
}
