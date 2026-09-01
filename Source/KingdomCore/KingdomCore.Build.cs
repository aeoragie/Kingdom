using UnrealBuildTool;

// 순수 C++ 시뮬레이션 코어. UE 헤더 의존 0개 — CoreMinimal.h가 한 줄이라도
// 들어가는 순간 분리는 끝난 것이다 (Docs/6.개발과_기술.md).
// 같은 소스를 CMake로도 빌드한다: Source/KingdomCore/CMakeLists.txt
public class KingdomCore : ModuleRules
{
	public KingdomCore(ReadOnlyTargetRules Target) : base(Target)
	{
		// UE 모듈 보일러플레이트(IMPLEMENT_MODULE) 없이 정적 라이브러리로만 링크
		bRequiresImplementModule = false;
		PCHUsage = PCHUsageMode.NoPCHs;
		bUseUnity = false;
		CppStandard = CppStandardVersion.Cpp20;
	}
}
