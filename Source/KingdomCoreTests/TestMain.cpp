#include "MiniTest.h"

#include "KingdomCore/Archive.h"
#include "KingdomCore/RandomStream.h"
#include "KingdomCore/SimulationClock.h"
#include "KingdomCore/Simulation.h"
#include "KingdomCore/Version.h"

#include <cstring>
#include <string>
#include <vector>

TEST_CASE(CoreVersion_IsNonEmpty)
{
	CHECK(Kingdom::CoreVersion() != nullptr);
	CHECK(std::strlen(Kingdom::CoreVersion()) > 0);
}

TEST_CASE(SimulationClock_StartsAtZero)
{
	Kingdom::FSimulationClock Clock;
	CHECK(Clock.CurrentTick() == 0);
}

TEST_CASE(SimulationClock_AdvanceAccumulates)
{
	Kingdom::FSimulationClock Clock;
	Clock.Advance(3);
	Clock.Advance(0);
	Clock.Advance(7);
	CHECK(Clock.CurrentTick() == 10);
}

// ---------------------------------------------------------------------------
// FRandomStream

TEST_CASE(RandomStream_SameSeedSameSequence)
{
	Kingdom::FRandomStream A(42);
	Kingdom::FRandomStream B(42);
	bool bAllEqual = true;
	for (int Index = 0; Index < 1000; ++Index)
	{
		bAllEqual = bAllEqual && (A.NextUInt64() == B.NextUInt64());
	}
	CHECK(bAllEqual);
	CHECK(A == B);
}

TEST_CASE(RandomStream_DifferentSeedDifferentSequence)
{
	Kingdom::FRandomStream A(1);
	Kingdom::FRandomStream B(2);
	CHECK(A.NextUInt64() != B.NextUInt64());
}

TEST_CASE(RandomStream_RandRangeStaysInBounds)
{
	Kingdom::FRandomStream Random(7);
	bool bInBounds = true;
	bool bSawMin = false;
	bool bSawMax = false;
	for (int Index = 0; Index < 10000; ++Index)
	{
		const Kingdom::int32 Value = Random.RandRange(-3, 3);
		bInBounds = bInBounds && (Value >= -3 && Value <= 3);
		bSawMin = bSawMin || (Value == -3);
		bSawMax = bSawMax || (Value == 3);
	}
	CHECK(bInBounds);
	CHECK(bSawMin);
	CHECK(bSawMax);
	CHECK(Random.RandRange(5, 5) == 5);
	CHECK(Random.RandRange(9, 1) == 9);
}

TEST_CASE(RandomStream_StateRoundTrip)
{
	Kingdom::FRandomStream Original(123);
	Original.NextUInt64();
	Original.NextUInt64();

	Kingdom::FRandomStream Restored(0);
	Restored.SetState(Original.GetState());
	CHECK(Restored == Original);
	CHECK(Restored.NextUInt64() == Original.NextUInt64());
}

// ---------------------------------------------------------------------------
// FSimulation (고정 틱 루프)

namespace
{

// 테스트용 시스템: 틱마다 난수를 하나 뽑아 누적하고, 호출 순서를 기록한다
class FProbeSystem : public Kingdom::ISimulationSystem
{
public:
	FProbeSystem(const char* InName, std::vector<std::string>* InCallLog, Kingdom::uint64* InAccumulator)
		: Name(InName)
		, CallLog(InCallLog)
		, Accumulator(InAccumulator)
	{
	}

	const char* GetName() const override { return Name; }

	void Tick(Kingdom::FSimulation& Simulation) override
	{
		if (CallLog)
		{
			CallLog->push_back(std::string(Name) + "@" + std::to_string(Simulation.CurrentTick()));
		}
		if (Accumulator)
		{
			*Accumulator = (*Accumulator * 31) ^ Simulation.GetRandom().NextUInt64();
		}
	}

private:
	const char* Name;
	std::vector<std::string>* CallLog;
	Kingdom::uint64* Accumulator;
};

} // namespace

TEST_CASE(Simulation_StartsAtTickZero)
{
	Kingdom::FSimulation Simulation(1);
	CHECK(Simulation.CurrentTick() == 0);
	CHECK(Simulation.GetSeed() == 1);
	Simulation.Advance(0);
	CHECK(Simulation.CurrentTick() == 0);
}

TEST_CASE(Simulation_RunsSystemsInOrderEveryTick)
{
	std::vector<std::string> CallLog;
	Kingdom::FSimulation Simulation(1);
	Simulation.AddSystem(std::make_unique<FProbeSystem>("A", &CallLog, nullptr));
	Simulation.AddSystem(std::make_unique<FProbeSystem>("B", &CallLog, nullptr));

	Simulation.Advance(2);

	const std::vector<std::string> Expected = {"A@1", "B@1", "A@2", "B@2"};
	CHECK(CallLog == Expected);
	CHECK(Simulation.CurrentTick() == 2);
}

TEST_CASE(Simulation_SameSeedSameResultAfter100Ticks)
{
	Kingdom::uint64 AccumulatorA = 0;
	Kingdom::uint64 AccumulatorB = 0;

	Kingdom::FSimulation A(2026);
	A.AddSystem(std::make_unique<FProbeSystem>("Probe", nullptr, &AccumulatorA));
	Kingdom::FSimulation B(2026);
	B.AddSystem(std::make_unique<FProbeSystem>("Probe", nullptr, &AccumulatorB));

	A.Advance(100);
	// 나눠서 진행해도 결과가 같아야 한다(배속은 틱 소비 속도일 뿐)
	B.Advance(30);
	B.Advance(1);
	B.Advance(69);

	CHECK(A.CurrentTick() == 100);
	CHECK(B.CurrentTick() == 100);
	CHECK(AccumulatorA == AccumulatorB);
	CHECK(A.GetRandom() == B.GetRandom());
}

// ---------------------------------------------------------------------------
// 커맨드 패턴

namespace
{

// 시험용 상태: 값 하나를 들고 있는 시스템. 상태는 커맨드로만 바뀐다
class FCounterSystem : public Kingdom::ISimulationSystem
{
public:
	static constexpr const char* SystemName = "Counter";

	const char* GetName() const override { return SystemName; }

	// 틱마다 난수를 하나 뽑아 누적한다(세이브 뒤에도 난수 흐름이 이어지는지 보려고)
	void Tick(Kingdom::FSimulation& Simulation) override
	{
		RandomTrail = (RandomTrail * 31) ^ Simulation.GetRandom().NextUInt64();
	}

	void Save(Kingdom::FArchiveWriter& Writer) const override
	{
		Writer.WriteInt64(Value);
		Writer.WriteUInt64(RandomTrail);
	}

	bool Load(Kingdom::FArchiveReader& Reader) override
	{
		const Kingdom::int64 LoadedValue = Reader.ReadInt64();
		const Kingdom::uint64 LoadedTrail = Reader.ReadUInt64();
		if (Reader.HasError())
		{
			return false;
		}
		Value = LoadedValue;
		RandomTrail = LoadedTrail;
		return true;
	}

	Kingdom::int64 Value = 0;
	Kingdom::uint64 RandomTrail = 0;
};

FCounterSystem* FindCounter(Kingdom::FSimulation& Simulation)
{
	return static_cast<FCounterSystem*>(Simulation.FindSystem(FCounterSystem::SystemName));
}

const FCounterSystem* FindCounter(const Kingdom::FSimulation& Simulation)
{
	return static_cast<const FCounterSystem*>(Simulation.FindSystem(FCounterSystem::SystemName));
}

// 시험용 커맨드: 카운터에 더한다. 결과가 음수가 되면 거부한다
class FAddCounterCommand : public Kingdom::ICommand
{
public:
	static constexpr Kingdom::uint32 TypeId = 1;

	FAddCounterCommand() = default;

	explicit FAddCounterCommand(Kingdom::int64 InAmount)
		: Amount(InAmount)
	{
	}

	static std::unique_ptr<Kingdom::ICommand> Create() { return std::make_unique<FAddCounterCommand>(); }

	Kingdom::uint32 GetTypeId() const override { return TypeId; }
	const char* GetName() const override { return "AddCounter"; }

	void Save(Kingdom::FArchiveWriter& Writer) const override { Writer.WriteInt64(Amount); }

	bool Load(Kingdom::FArchiveReader& Reader) override
	{
		Amount = Reader.ReadInt64();
		return !Reader.HasError();
	}

	Kingdom::FCommandValidation Validate(const Kingdom::FSimulation& Simulation) const override
	{
		const FCounterSystem* Counter = FindCounter(Simulation);
		if (!Counter)
		{
			return Kingdom::FCommandValidation::Fail("no counter system");
		}
		if (Counter->Value + Amount < 0)
		{
			return Kingdom::FCommandValidation::Fail("counter would go negative");
		}
		return Kingdom::FCommandValidation::Ok();
	}

	void Apply(Kingdom::FSimulation& Simulation) override
	{
		FindCounter(Simulation)->Value += Amount;
	}

private:
	Kingdom::int64 Amount = 0;
};

std::unique_ptr<Kingdom::FSimulation> MakeCounterSimulation(Kingdom::uint64 Seed)
{
	auto Simulation = std::make_unique<Kingdom::FSimulation>(Seed);
	Simulation->AddSystem(std::make_unique<FCounterSystem>());
	Simulation->RegisterCommandType(FAddCounterCommand::TypeId, &FAddCounterCommand::Create);
	return Simulation;
}

} // namespace

TEST_CASE(Command_AppliedOnNextTickNotImmediately)
{
	auto Simulation = MakeCounterSimulation(1);
	Simulation->SubmitCommand(std::make_unique<FAddCounterCommand>(5));

	CHECK(FindCounter(*Simulation)->Value == 0);
	CHECK(Simulation->GetPendingCommandCount() == 1);

	Simulation->Advance(1);

	CHECK(FindCounter(*Simulation)->Value == 5);
	CHECK(Simulation->GetPendingCommandCount() == 0);
	CHECK(Simulation->GetCommandLog().size() == 1);
	CHECK(Simulation->GetCommandLog()[0].AppliedTick == 1);
	CHECK(Simulation->GetCommandLog()[0].bAccepted);
}

TEST_CASE(Command_InvalidIsRejectedAndStateUnchanged)
{
	auto Simulation = MakeCounterSimulation(1);
	Simulation->SubmitCommand(std::make_unique<FAddCounterCommand>(-3));
	Simulation->Advance(1);

	CHECK(FindCounter(*Simulation)->Value == 0);
	CHECK(Simulation->GetCommandLog().size() == 1);
	CHECK(!Simulation->GetCommandLog()[0].bAccepted);
	CHECK(Simulation->GetCommandLog()[0].Reason == "counter would go negative");
}

TEST_CASE(Command_ValidatedAtApplyTimeInSubmitOrder)
{
	// 둘 다 제출 시점에는 유효하지만, 앞 커맨드가 적용된 뒤에는 둘째가 무효가 된다
	auto Simulation = MakeCounterSimulation(1);
	Simulation->SubmitCommand(std::make_unique<FAddCounterCommand>(2));
	Simulation->Advance(1);

	const Kingdom::uint64 First = Simulation->SubmitCommand(std::make_unique<FAddCounterCommand>(-2));
	const Kingdom::uint64 Second = Simulation->SubmitCommand(std::make_unique<FAddCounterCommand>(-1));
	CHECK(Second == First + 1);
	Simulation->Advance(1);

	const auto& Log = Simulation->GetCommandLog();
	CHECK(Log.size() == 3);
	CHECK(Log[1].Sequence == First && Log[1].bAccepted);
	CHECK(Log[2].Sequence == Second && !Log[2].bAccepted);
	CHECK(FindCounter(*Simulation)->Value == 0);
}

TEST_CASE(Command_NoSystemIsRejected)
{
	Kingdom::FSimulation Simulation(1);
	Simulation.SubmitCommand(std::make_unique<FAddCounterCommand>(1));
	Simulation.Advance(1);

	CHECK(Simulation.GetCommandLog().size() == 1);
	CHECK(!Simulation.GetCommandLog()[0].bAccepted);
}

TEST_CASE(Command_SameCommandsSameResult)
{
	auto A = MakeCounterSimulation(9);
	auto B = MakeCounterSimulation(9);
	const Kingdom::int64 Amounts[] = {3, -1, 7, -20, 4};
	for (Kingdom::int64 Amount : Amounts)
	{
		A->SubmitCommand(std::make_unique<FAddCounterCommand>(Amount));
		B->SubmitCommand(std::make_unique<FAddCounterCommand>(Amount));
		A->Advance(2);
		B->Advance(1);
		B->Advance(1);
	}
	CHECK(FindCounter(*A)->Value == FindCounter(*B)->Value);
	CHECK(FindCounter(*A)->Value == 13);
	CHECK(A->GetCommandLog().size() == B->GetCommandLog().size());
}

// ---------------------------------------------------------------------------
// 직렬화(FArchiveWriter / FArchiveReader)

TEST_CASE(Archive_RoundTripAllTypes)
{
	Kingdom::FArchiveWriter Writer;
	Writer.WriteUInt8(200);
	Writer.WriteUInt32(0xDEADBEEFu);
	Writer.WriteUInt64(0x0123456789ABCDEFull);
	Writer.WriteInt32(-7);
	Writer.WriteInt64(-1234567890123ll);
	Writer.WriteBool(true);
	Writer.WriteString("왕국");
	Writer.WriteBytes({1, 2, 3});

	const std::vector<Kingdom::uint8> Bytes = Writer.TakeBuffer();
	Kingdom::FArchiveReader Reader(Bytes);
	CHECK(Reader.ReadUInt8() == 200);
	CHECK(Reader.ReadUInt32() == 0xDEADBEEFu);
	CHECK(Reader.ReadUInt64() == 0x0123456789ABCDEFull);
	CHECK(Reader.ReadInt32() == -7);
	CHECK(Reader.ReadInt64() == -1234567890123ll);
	CHECK(Reader.ReadBool());
	CHECK(Reader.ReadString() == "왕국");
	CHECK((Reader.ReadBytes() == std::vector<Kingdom::uint8>{1, 2, 3}));
	CHECK(!Reader.HasError());
	CHECK(Reader.IsAtEnd());
}

TEST_CASE(Archive_LittleEndianLayout)
{
	Kingdom::FArchiveWriter Writer;
	Writer.WriteUInt32(0x04030201u);
	CHECK((Writer.GetBuffer() == std::vector<Kingdom::uint8>{1, 2, 3, 4}));
}

TEST_CASE(Archive_ReadingPastEndSetsError)
{
	const std::vector<Kingdom::uint8> Bytes = {1, 2};
	Kingdom::FArchiveReader Reader(Bytes);
	CHECK(Reader.ReadUInt32() == 0);
	CHECK(Reader.HasError());
	CHECK(Reader.ReadUInt8() == 0);
}

// ---------------------------------------------------------------------------
// 세이브 / 로드

TEST_CASE(SaveLoad_ContinuesExactlyLikeNeverSaved)
{
	// 같은 지점에서: 그대로 N틱 vs 저장·로드한 뒤 N틱. 결과가 같아야 한다
	auto Original = MakeCounterSimulation(77);
	for (int Step = 0; Step < 10; ++Step)
	{
		Original->SubmitCommand(std::make_unique<FAddCounterCommand>(Step - 3));
		Original->Advance(5);
	}

	const std::vector<Kingdom::uint8> Snapshot = Original->Save();

	auto Restored = MakeCounterSimulation(0);	// 시드가 달라도 세이브가 덮어쓴다
	CHECK(Restored->Load(Snapshot));
	CHECK(Restored->CurrentTick() == Original->CurrentTick());
	CHECK(Restored->GetSeed() == 77);

	for (int Step = 0; Step < 10; ++Step)
	{
		Original->SubmitCommand(std::make_unique<FAddCounterCommand>(Step));
		Restored->SubmitCommand(std::make_unique<FAddCounterCommand>(Step));
		Original->Advance(7);
		Restored->Advance(7);
	}

	CHECK(Restored->CurrentTick() == Original->CurrentTick());
	CHECK(FindCounter(*Restored)->Value == FindCounter(*Original)->Value);
	CHECK(FindCounter(*Restored)->RandomTrail == FindCounter(*Original)->RandomTrail);
	CHECK(Restored->GetRandom() == Original->GetRandom());
}

TEST_CASE(SaveLoad_PendingCommandSurvives)
{
	auto Original = MakeCounterSimulation(1);
	Original->SubmitCommand(std::make_unique<FAddCounterCommand>(9));
	const std::vector<Kingdom::uint8> Snapshot = Original->Save();

	auto Restored = MakeCounterSimulation(1);
	CHECK(Restored->Load(Snapshot));
	CHECK(Restored->GetPendingCommandCount() == 1);
	Restored->Advance(1);
	CHECK(FindCounter(*Restored)->Value == 9);
}

TEST_CASE(SaveLoad_LogIsSinceLastSave)
{
	auto Simulation = MakeCounterSimulation(1);
	Simulation->SubmitCommand(std::make_unique<FAddCounterCommand>(1));
	Simulation->SubmitCommand(std::make_unique<FAddCounterCommand>(-5));
	Simulation->Advance(1);
	CHECK(Simulation->GetCommandLog().size() == 2);

	const std::vector<Kingdom::uint8> Snapshot = Simulation->Save();
	CHECK(Simulation->GetCommandLog().empty());

	// 세이브 안에는 저장 직전까지의 로그가 들어 있다(거부 이유 포함)
	auto Restored = MakeCounterSimulation(1);
	CHECK(Restored->Load(Snapshot));
	CHECK(Restored->GetCommandLog().size() == 2);
	CHECK(!Restored->GetCommandLog()[1].bAccepted);
	CHECK(Restored->GetCommandLog()[1].Reason == "counter would go negative");
}

TEST_CASE(SaveLoad_RejectsBrokenData)
{
	auto Original = MakeCounterSimulation(1);
	Original->Advance(3);
	std::vector<Kingdom::uint8> Snapshot = Original->Save();

	auto Target = MakeCounterSimulation(1);

	std::vector<Kingdom::uint8> BadMagic = Snapshot;
	BadMagic[0] ^= 0xFF;
	CHECK(!Target->Load(BadMagic));

	std::vector<Kingdom::uint8> Truncated(Snapshot.begin(), Snapshot.end() - 1);
	CHECK(!Target->Load(Truncated));

	std::vector<Kingdom::uint8> Extra = Snapshot;
	Extra.push_back(0);
	CHECK(!Target->Load(Extra));

	// 실패한 뒤에도 원래 상태 그대로
	CHECK(Target->CurrentTick() == 0);
	CHECK(Target->Load(Snapshot));
	CHECK(Target->CurrentTick() == 3);
}

TEST_CASE(SaveLoad_RejectsUnknownCommandType)
{
	auto Original = MakeCounterSimulation(1);
	Original->SubmitCommand(std::make_unique<FAddCounterCommand>(1));
	const std::vector<Kingdom::uint8> Snapshot = Original->Save();

	// 커맨드 종류를 등록하지 않은 시뮬레이션
	Kingdom::FSimulation Target(1);
	Target.AddSystem(std::make_unique<FCounterSystem>());
	CHECK(!Target.Load(Snapshot));
}

TEST_CASE(SaveLoad_RejectsDifferentSystems)
{
	auto Original = MakeCounterSimulation(1);
	const std::vector<Kingdom::uint8> Snapshot = Original->Save();

	Kingdom::FSimulation NoSystems(1);
	NoSystems.RegisterCommandType(FAddCounterCommand::TypeId, &FAddCounterCommand::Create);
	CHECK(!NoSystems.Load(Snapshot));
}

// 인자 없이: 전체. 인자 하나: 이름에 그 글자가 든 테스트만 (예: KingdomCoreTests.exe SaveLoad)
int main(int ArgumentCount, char** Arguments)
{
	return MiniTest::RunAll(ArgumentCount > 1 ? Arguments[1] : nullptr);
}
