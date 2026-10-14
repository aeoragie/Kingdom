#pragma once

#include "KingdomCore/CoreTypes.h"

namespace Kingdom
{

// 결정론 난수. 확률 판정(모사, 강화, 전투)은 전부 이것만 쓴다.
// 상태(FState)를 그대로 꺼내고 넣을 수 있어 세이브 스냅샷에 함께 저장한다.
// 알고리즘: SplitMix64로 시드를 펼치고 xoshiro256**로 뽑는다.
class FRandomStream
{
public:
	using FState = std::array<uint64, 4>;

	KINGDOMCORE_API explicit FRandomStream(uint64 Seed = 0);

	KINGDOMCORE_API void Reseed(uint64 Seed);

	KINGDOMCORE_API uint64 NextUInt64();
	KINGDOMCORE_API uint32 NextUInt32();

	// [Min, Max] 정수 하나. 치우침 없이 뽑는다. Min > Max면 Min을 돌려준다.
	KINGDOMCORE_API int32 RandRange(int32 Min, int32 Max);

	const FState& GetState() const { return State; }
	void SetState(const FState& InState) { State = InState; }

	bool operator==(const FRandomStream& Other) const { return State == Other.State; }

private:
	FState State{};
};

} // namespace Kingdom
