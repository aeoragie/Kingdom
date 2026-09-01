using UnrealBuildTool;

public class KingdomTarget : TargetRules
{
	public KingdomTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("KingdomGame");
	}
}
