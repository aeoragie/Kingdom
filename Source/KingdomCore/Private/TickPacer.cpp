#include "KingdomCore/TickPacer.h"

namespace Kingdom
{

FTickPacer::FTickPacer(uint64 InTickIntervalMicroseconds, uint64 InMaxElapsedMicroseconds)
	: TickIntervalMicroseconds(std::max<uint64>(InTickIntervalMicroseconds, 1))
	, MaxElapsedMicroseconds(InMaxElapsedMicroseconds)
{
}

void FTickPacer::SetSpeed(uint32 Multiplier)
{
	Speed = std::max<uint32>(Multiplier, 1);
}

uint32 FTickPacer::Consume(uint64 ElapsedMicroseconds)
{
	if (bPaused)
	{
		return 0;
	}

	if (MaxElapsedMicroseconds > 0)
	{
		ElapsedMicroseconds = std::min(ElapsedMicroseconds, MaxElapsedMicroseconds);
	}

	AccumulatedMicroseconds += ElapsedMicroseconds * Speed;

	// 상한이 없을 때 한 번에 uint32를 넘는 틱이 나오면 남은 것은 다음 호출로 넘긴다
	const uint64 TickCount = std::min<uint64>(
		AccumulatedMicroseconds / TickIntervalMicroseconds,
		std::numeric_limits<uint32>::max());
	AccumulatedMicroseconds -= TickCount * TickIntervalMicroseconds;

	return static_cast<uint32>(TickCount);
}

} // namespace Kingdom
