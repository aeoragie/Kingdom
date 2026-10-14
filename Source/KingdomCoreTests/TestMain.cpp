#include "MiniTest.h"

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
	void Tick(Kingdom::FSimulation&) override {}

	Kingdom::int64 Value = 0;
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
	explicit FAddCounterCommand(Kingdom::int64 InAmount)
		: Amount(InAmount)
	{
	}

	const char* GetName() const override { return "AddCounter"; }

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
	CHECK(std::strcmp(Simulation->GetCommandLog()[0].Reason, "counter would go negative") == 0);
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

int main()
{
	return MiniTest::RunAll();
}
