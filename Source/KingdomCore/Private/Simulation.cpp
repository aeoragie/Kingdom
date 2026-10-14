#include "KingdomCore/Simulation.h"

namespace Kingdom
{

FSimulation::FSimulation(uint64 InSeed)
	: Seed(InSeed)
	, Random(InSeed)
{
}

FSimulation::~FSimulation() = default;

void FSimulation::AddSystem(std::unique_ptr<ISimSystem> System)
{
	if (System)
	{
		Systems.push_back(std::move(System));
	}
}

void FSimulation::Advance(uint32 TickCount)
{
	for (uint32 Index = 0; Index < TickCount; ++Index)
	{
		TickOnce();
	}
}

void FSimulation::TickOnce()
{
	// 시계를 먼저 올린다. 시스템은 "지금 처리 중인 틱 번호"를 CurrentTick()으로 본다(1부터)
	Clock.Advance(1);

	for (const std::unique_ptr<ISimSystem>& System : Systems)
	{
		System->Tick(*this);
	}
}

} // namespace Kingdom
