#pragma once

#include "KingdomCore/CoreTypes.h"

namespace Kingdom
{

// 게임 시간은 정수 틱으로만 흐른다. 부동소수점 누적 금지.
// 틱 하나가 몇 초인지는 코어가 모른다 — 바깥(뷰/설정)이 정한다.
// 배속은 틱 간격이 아니라 틱 소비 속도로 구현한다.
class FSimulationClock
{
public:
	uint64 CurrentTick() const { return Tick; }

	void Advance(uint32 TickCount) { Tick += TickCount; }

	// 세이브를 불러올 때만 쓴다
	void Restore(uint64 InTick) { Tick = InTick; }

private:
	uint64 Tick = 0;
};

} // namespace Kingdom
