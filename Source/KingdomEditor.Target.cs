using UnrealBuildTool;

public class KingdomEditorTarget : TargetRules
{
	public KingdomEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("KingdomGame");
	}
}
