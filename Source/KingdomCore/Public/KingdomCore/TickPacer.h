#pragma once

#include "KingdomCore/CoreTypes.h"

namespace Kingdom
{

// 바깥(뷰, 장래의 서버)의 실제 시간을 "이번에 돌릴 틱 수"로 바꾼다.
// 시뮬레이션 상태가 아니므로 세이브에 넣지 않는다.
//
// 시간은 정수 마이크로초로만 받는다(부동소수점 누적 없음).
// 틱 간격은 바깥이 정해 넘긴다. 코어의 시뮬레이션은 여전히 틱이 몇 초인지 모른다.
//
// 배속은 틱 간격이 아니라 흐른 시간에 곱한다(틱 소비 속도).
// 일시정지 중에는 시간을 세지 않으므로, 풀어도 밀린 틱이 없다.
//
// 한 번에 받는 시간에는 상한이 있다(기본 0.25초).
// 디버거 중단점·절전처럼 의도치 않게 멈춰 있던 시간은 게임 시간으로 치지 않는다.
// 다시 움직이면 멈추기 직전 상황에서 이어진다.
// 상한은 배속을 곱하기 전의 실제 시간에 건다. 0이면 상한 없음(장래의 서버처럼 시간을 버리면 안 될 때).
class FTickPacer
{
public:
	static constexpr uint64 DefaultMaxElapsedMicroseconds = 250000;

	explicit KINGDOMCORE_API FTickPacer(
		uint64 TickIntervalMicroseconds,
		uint64 MaxElapsedMicroseconds = DefaultMaxElapsedMicroseconds);

	// 0은 1로 본다. 어떤 배속을 허용할지는 바깥이 정한다.
	KINGDOMCORE_API void SetSpeed(uint32 Multiplier);
	uint32 GetSpeed() const { return Speed; }

	void SetPaused(bool bInPaused) { bPaused = bInPaused; }
	bool IsPaused() const { return bPaused; }

	// 흐른 시간을 받아 이번에 돌릴 틱 수를 돌려준다.
	KINGDOMCORE_API uint32 Consume(uint64 ElapsedMicroseconds);

private:
	uint64 TickIntervalMicroseconds = 1;
	uint64 MaxElapsedMicroseconds = 0;
	uint64 AccumulatedMicroseconds = 0;
	uint32 Speed = 1;
	bool bPaused = false;
};

} // namespace Kingdom
