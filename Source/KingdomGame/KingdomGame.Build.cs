using UnrealBuildTool;

// UE5 뷰 레이어. 그리고 입력받는 역할만 한다.
// 코어 상태를 직접 수정하지 않는다 — 커맨드를 코어에 제출하고 결과를 읽기만.
public class KingdomGame : ModuleRules
{
	public KingdomGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"KingdomCore"
		});
	}
}
