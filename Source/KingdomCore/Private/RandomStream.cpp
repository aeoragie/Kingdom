#include "KingdomCore/RandomStream.h"

namespace Kingdom
{

namespace
{

uint64 SplitMix64(uint64& X)
{
	X += 0x9E3779B97F4A7C15ull;
	uint64 Z = X;
	Z = (Z ^ (Z >> 30)) * 0xBF58476D1CE4E5B9ull;
	Z = (Z ^ (Z >> 27)) * 0x94D049BB133111EBull;
	return Z ^ (Z >> 31);
}

uint64 RotateLeft(uint64 X, int32 K)
{
	return (X << K) | (X >> (64 - K));
}

} // namespace

FRandomStream::FRandomStream(uint64 Seed)
{
	Reseed(Seed);
}

void FRandomStream::Reseed(uint64 Seed)
{
	uint64 X = Seed;
	for (uint64& Word : State)
	{
		Word = SplitMix64(X);
	}
}

uint64 FRandomStream::NextUInt64()
{
	const uint64 Result = RotateLeft(State[1] * 5, 7) * 9;
	const uint64 T = State[1] << 17;

	State[2] ^= State[0];
	State[3] ^= State[1];
	State[1] ^= State[2];
	State[0] ^= State[3];
	State[2] ^= T;
	State[3] = RotateLeft(State[3], 45);

	return Result;
}

uint32 FRandomStream::NextUInt32()
{
	return static_cast<uint32>(NextUInt64() >> 32);
}

int32 FRandomStream::RandRange(int32 Min, int32 Max)
{
	if (Min >= Max)
	{
		return Min;
	}

	const uint64 Span = static_cast<uint64>(static_cast<int64>(Max) - static_cast<int64>(Min)) + 1;

	// 치우침 제거: Span의 배수가 되는 구간 밖의 값은 버리고 다시 뽑는다
	const uint64 Limit = UINT64_MAX - (UINT64_MAX % Span);
	uint64 Value = NextUInt64();
	while (Value >= Limit)
	{
		Value = NextUInt64();
	}

	return static_cast<int32>(static_cast<int64>(Min) + static_cast<int64>(Value % Span));
}

} // namespace Kingdom
