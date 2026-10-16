#pragma once

// 최소 테스트 하네스. 외부 의존 없이 Phase 0 게이트("UE 없이 콘솔에서 테스트가
// 돈다")를 넘기기 위한 것. 본격적인 프레임워크가 필요해지면 교체한다.

#include <cstdio>
#include <cstring>
#include <vector>

namespace MiniTest
{

struct FTest
{
	const char* Name;
	void (*Func)();
};

inline std::vector<FTest>& Registry()
{
	static std::vector<FTest> Tests;
	return Tests;
}

inline int FailedCheckCount = 0;

struct FRegistrar
{
	FRegistrar(const char* Name, void (*Func)()) { Registry().push_back({Name, Func}); }
};

// Filter가 있으면 이름에 그 글자가 든 테스트만 돈다(예: "SaveLoad").
// 맞는 테스트가 하나도 없으면 실패로 끝낸다(오타로 아무것도 안 돌고 통과하는 것을 막는다).
inline int RunAll(const char* Filter = nullptr)
{
	int RunTests = 0;
	int FailedTests = 0;
	for (const FTest& Test : Registry())
	{
		if (Filter && !std::strstr(Test.Name, Filter))
		{
			continue;
		}

		++RunTests;
		const int Before = FailedCheckCount;
		Test.Func();
		const bool bPassed = (FailedCheckCount == Before);
		std::printf("[%s] %s\n", bPassed ? "PASS" : "FAIL", Test.Name);
		if (!bPassed)
		{
			++FailedTests;
		}
	}

	if (RunTests == 0)
	{
		std::printf("No tests match \"%s\"\n", Filter ? Filter : "");
		return 1;
	}

	std::printf("%d tests, %d failed\n", RunTests, FailedTests);
	return FailedTests == 0 ? 0 : 1;
}

} // namespace MiniTest

#define TEST_CASE(Name) \
	static void Name(); \
	static MiniTest::FRegistrar Registrar_##Name(#Name, &Name); \
	static void Name()

#define CHECK(Expr) \
	do \
	{ \
		if (!(Expr)) \
		{ \
			std::printf("  CHECK failed: %s (%s:%d)\n", #Expr, __FILE__, __LINE__); \
			++MiniTest::FailedCheckCount; \
		} \
	} while (0)
