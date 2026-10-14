#include "KingdomCore/Simulation.h"

#include "KingdomCore/Archive.h"

#include <cstring>

namespace Kingdom
{

namespace
{

constexpr uint32 SaveMagic = 0x5641534Bu;	// "KSAV" (리틀 엔디언)
constexpr uint32 SaveVersion = 1;

} // namespace

FSimulation::FSimulation(uint64 InSeed)
	: Seed(InSeed)
	, Random(InSeed)
{
}

FSimulation::~FSimulation() = default;

void FSimulation::AddSystem(unique_ptr<ISimulationSystem> System)
{
	if (System)
	{
		Systems.push_back(std::move(System));
	}
}

ISimulationSystem* FSimulation::FindSystem(const char* Name)
{
	if (!Name)
	{
		return nullptr;
	}
	for (const unique_ptr<ISimulationSystem>& System : Systems)
	{
		if (std::strcmp(System->GetName(), Name) == 0)
		{
			return System.get();
		}
	}
	return nullptr;
}

const ISimulationSystem* FSimulation::FindSystem(const char* Name) const
{
	return const_cast<FSimulation*>(this)->FindSystem(Name);
}

void FSimulation::RegisterCommandType(uint32 TypeId, FCommandFactory Factory)
{
	for (FCommandType& Type : CommandTypes)
	{
		if (Type.TypeId == TypeId)
		{
			Type.Factory = Factory;
			return;
		}
	}
	CommandTypes.push_back({TypeId, Factory});
}

uint64 FSimulation::SubmitCommand(unique_ptr<ICommand> Command)
{
	const uint64 Sequence = NextSequence++;
	if (Command)
	{
		PendingCommands.push_back({Sequence, shared_ptr<ICommand>(std::move(Command))});
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

	for (const unique_ptr<ISimulationSystem>& System : Systems)
	{
		System->Tick(*this);
	}
}

void FSimulation::ProcessPendingCommands()
{
	// 처리 중에 새로 제출된 커맨드는 다음 틱으로 넘긴다
	vector<FPendingCommand> Batch;
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

shared_ptr<ICommand> FSimulation::CreateCommand(uint32 TypeId) const
{
	for (const FCommandType& Type : CommandTypes)
	{
		if (Type.TypeId == TypeId && Type.Factory)
		{
			return shared_ptr<ICommand>(Type.Factory());
		}
	}
	return nullptr;
}

void FSimulation::SaveCommand(FArchiveWriter& Writer, const ICommand& Command) const
{
	FArchiveWriter Payload;
	Command.Save(Payload);

	Writer.WriteUInt32(Command.GetTypeId());
	Writer.WriteBytes(Payload.GetBuffer());
}

shared_ptr<ICommand> FSimulation::LoadCommand(FArchiveReader& Reader) const
{
	const uint32 TypeId = Reader.ReadUInt32();
	const vector<uint8> PayloadBytes = Reader.ReadBytes();
	if (Reader.HasError())
	{
		return nullptr;
	}

	shared_ptr<ICommand> Command = CreateCommand(TypeId);
	if (!Command)
	{
		return nullptr;
	}

	FArchiveReader Payload(PayloadBytes);
	if (!Command->Load(Payload) || Payload.HasError() || !Payload.IsAtEnd())
	{
		return nullptr;
	}
	return Command;
}

vector<uint8> FSimulation::Save()
{
	FArchiveWriter Writer;

	Writer.WriteUInt32(SaveMagic);
	Writer.WriteUInt32(SaveVersion);

	Writer.WriteUInt64(Seed);
	Writer.WriteUInt64(Clock.CurrentTick());
	for (uint64 Word : Random.GetState())
	{
		Writer.WriteUInt64(Word);
	}
	Writer.WriteUInt64(NextSequence);

	// 시스템: 이름 + 상태 바이트(길이 포함). 불러올 때 이름으로 짝을 맞추고 길이로 검증한다
	Writer.WriteUInt32(static_cast<uint32>(Systems.size()));
	for (const unique_ptr<ISimulationSystem>& System : Systems)
	{
		FArchiveWriter SystemWriter;
		System->Save(SystemWriter);

		Writer.WriteString(System->GetName());
		Writer.WriteBytes(SystemWriter.GetBuffer());
	}

	Writer.WriteUInt32(static_cast<uint32>(PendingCommands.size()));
	for (const FPendingCommand& Pending : PendingCommands)
	{
		Writer.WriteUInt64(Pending.Sequence);
		SaveCommand(Writer, *Pending.Command);
	}

	Writer.WriteUInt32(static_cast<uint32>(CommandLog.size()));
	for (const FCommandRecord& Record : CommandLog)
	{
		Writer.WriteUInt64(Record.Sequence);
		Writer.WriteUInt64(Record.AppliedTick);
		Writer.WriteBool(Record.bAccepted);
		Writer.WriteString(Record.Reason);
		SaveCommand(Writer, *Record.Command);
	}

	// 이 세이브 이후 쌓이는 로그가 "마지막 세이브 이후 커맨드"가 된다
	CommandLog.clear();

	return Writer.TakeBuffer();
}

bool FSimulation::Load(const vector<uint8>& Bytes)
{
	FArchiveReader Reader(Bytes);

	if (Reader.ReadUInt32() != SaveMagic || Reader.ReadUInt32() != SaveVersion)
	{
		return false;
	}

	// 전부 임시 값에 읽고, 끝까지 문제가 없을 때만 실제 상태에 넣는다(실패해도 상태를 망치지 않게)
	const uint64 LoadedSeed = Reader.ReadUInt64();
	const uint64 LoadedTick = Reader.ReadUInt64();
	FRandomStream::FState LoadedRandomState{};
	for (uint64& Word : LoadedRandomState)
	{
		Word = Reader.ReadUInt64();
	}
	const uint64 LoadedNextSequence = Reader.ReadUInt64();

	const uint32 SystemCount = Reader.ReadUInt32();
	if (Reader.HasError() || SystemCount != Systems.size())
	{
		return false;
	}

	vector<vector<uint8>> SystemStates;
	SystemStates.reserve(SystemCount);
	for (uint32 Index = 0; Index < SystemCount; ++Index)
	{
		const string Name = Reader.ReadString();
		vector<uint8> State = Reader.ReadBytes();
		if (Reader.HasError() || Name != Systems[Index]->GetName())
		{
			return false;
		}
		SystemStates.push_back(std::move(State));
	}

	const uint32 PendingCount = Reader.ReadUInt32();
	vector<FPendingCommand> LoadedPending;
	for (uint32 Index = 0; Index < PendingCount && !Reader.HasError(); ++Index)
	{
		FPendingCommand Pending;
		Pending.Sequence = Reader.ReadUInt64();
		Pending.Command = LoadCommand(Reader);
		if (!Pending.Command)
		{
			return false;
		}
		LoadedPending.push_back(std::move(Pending));
	}

	const uint32 LogCount = Reader.ReadUInt32();
	vector<FCommandRecord> LoadedLog;
	for (uint32 Index = 0; Index < LogCount && !Reader.HasError(); ++Index)
	{
		FCommandRecord Record;
		Record.Sequence = Reader.ReadUInt64();
		Record.AppliedTick = Reader.ReadUInt64();
		Record.bAccepted = Reader.ReadBool();
		Record.Reason = Reader.ReadString();
		Record.Command = LoadCommand(Reader);
		if (!Record.Command)
		{
			return false;
		}
		LoadedLog.push_back(std::move(Record));
	}

	if (Reader.HasError() || !Reader.IsAtEnd())
	{
		return false;
	}

	// 시스템 상태는 각 시스템이 읽는다. 하나라도 실패하면 전체 실패
	// (시스템 Load가 일부만 바꾸고 실패할 수 있으므로, 시스템은 성공할 때만 상태를 바꾸게 구현한다)
	for (uint32 Index = 0; Index < SystemCount; ++Index)
	{
		FArchiveReader SystemReader(SystemStates[Index]);
		if (!Systems[Index]->Load(SystemReader) || SystemReader.HasError() || !SystemReader.IsAtEnd())
		{
			return false;
		}
	}

	Seed = LoadedSeed;
	Clock.Restore(LoadedTick);
	Random.SetState(LoadedRandomState);
	NextSequence = LoadedNextSequence;
	PendingCommands = std::move(LoadedPending);
	CommandLog = std::move(LoadedLog);
	return true;
}

} // namespace Kingdom
