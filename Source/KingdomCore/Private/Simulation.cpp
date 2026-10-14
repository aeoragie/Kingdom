#include "KingdomCore/Simulation.h"

#include <cstring>

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

ISimSystem* FSimulation::FindSystem(const char* Name)
{
	if (!Name)
	{
		return nullptr;
	}
	for (const std::unique_ptr<ISimSystem>& System : Systems)
	{
		if (std::strcmp(System->GetName(), Name) == 0)
		{
			return System.get();
		}
	}
	return nullptr;
}

const ISimSystem* FSimulation::FindSystem(const char* Name) const
{
	return const_cast<FSimulation*>(this)->FindSystem(Name);
}

uint64 FSimulation::SubmitCommand(std::unique_ptr<ICommand> Command)
{
	const uint64 Sequence = NextSequence++;
	if (Command)
	{
		PendingCommands.push_back({Sequence, std::shared_ptr<ICommand>(std::move(Command))});
	}
	return Sequence;
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
	Clock.Advance(1);
	ProcessPendingCommands();

	for (const std::unique_ptr<ISimSystem>& System : Systems)
	{
		System->Tick(*this);
	}
}

void FSimulation::ProcessPendingCommands()
{
	// 처리 중에 새로 제출된 커맨드는 다음 틱으로 넘긴다
	std::vector<FPendingCommand> Batch;
	Batch.swap(PendingCommands);

	for (FPendingCommand& Pending : Batch)
	{
		FCommandRecord Record;
		Record.Sequence = Pending.Sequence;
		Record.AppliedTick = CurrentTick();
		Record.Command = Pending.Command;

		const FCommandValidation Validation = Pending.Command->Validate(*this);
		Record.bAccepted = Validation.bValid;
		Record.Reason = Validation.Reason;

		if (Validation.bValid)
		{
			Pending.Command->Apply(*this);
		}

		CommandLog.push_back(std::move(Record));
	}
}

} // namespace Kingdom
