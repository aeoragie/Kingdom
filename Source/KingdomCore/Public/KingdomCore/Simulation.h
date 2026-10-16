#pragma once

#include "KingdomCore/Command.h"
#include "KingdomCore/CoreTypes.h"
#include "KingdomCore/RandomStream.h"
#include "KingdomCore/SimulationClock.h"

namespace Kingdom
{

class FArchiveReader;
class FArchiveWriter;
class FSimulation;

// 틱마다 한 번 도는 규칙 단위(생산, 이동, 전투 …).
// 시스템은 등록한 순서대로 돈다. 순서도 결정론의 일부다.
// 상태를 가진 시스템은 Save/Load를 구현한다(상태가 없으면 기본 구현 그대로).
class ISimulationSystem
{
public:
	virtual ~ISimulationSystem() = default;

	virtual const char* GetName() const = 0;
	virtual void Tick(FSimulation& Simulation) = 0;

	virtual void Save(FArchiveWriter& Writer) const { (void)Writer; }
	virtual bool Load(FArchiveReader& Reader) { (void)Reader; return true; }
};

// 고정 틱 시뮬레이션 루프.
// 코어는 "틱 N번 진행"만 안다. 틱 하나가 몇 초인지, 배속과 일시정지는 바깥(뷰)이 정한다.
//
// 한 틱의 순서:
//   1. 시계를 1 올린다(시스템과 커맨드는 CurrentTick()으로 지금 처리 중인 틱을 본다, 1부터)
//   2. 대기 중인 커맨드를 제출 순서대로 Validate → Apply
//   3. 시스템을 등록 순서대로 Tick
//
// 세이브 / 로드:
//   Save()는 전체 상태 스냅샷을 바이트로 돌려준다. 불러오면 그 상태에서 바로 이어 돈다.
//   함께 담는 커맨드 로그는 "마지막 세이브 이후" 것이고, 저장한 뒤 로그는 비운다.
//   Load()는 같은 시스템을 같은 순서로 등록하고 커맨드 종류를 등록해 둔 시뮬레이션에서 부른다.
class FSimulation
{
public:
	explicit KINGDOMCORE_API FSimulation(uint64 Seed);
	KINGDOMCORE_API ~FSimulation();

	FSimulation(const FSimulation&) = delete;
	FSimulation& operator=(const FSimulation&) = delete;

	KINGDOMCORE_API void AddSystem(unique_ptr<ISimulationSystem> System);

	// 이름으로 시스템을 찾는다. 없으면 nullptr. 커맨드가 자기 대상 시스템을 찾을 때 쓴다.
	KINGDOMCORE_API ISimulationSystem* FindSystem(const char* Name);
	KINGDOMCORE_API const ISimulationSystem* FindSystem(const char* Name) const;

	// 커맨드 종류를 등록한다. 세이브를 불러올 때 종류 번호로 커맨드를 다시 만든다.
	KINGDOMCORE_API void RegisterCommandType(uint32 TypeId, FCommandFactory Factory);

	// 커맨드를 제출한다. 다음 틱 처음에 처리된다. 돌려주는 값은 제출 순번.
	KINGDOMCORE_API uint64 SubmitCommand(unique_ptr<ICommand> Command);

	// TickCount만큼 틱을 진행한다.
	KINGDOMCORE_API void Advance(uint32 TickCount);

	// 전체 상태 스냅샷. 저장한 뒤 커맨드 로그를 비운다.
	KINGDOMCORE_API vector<uint8> Save();

	// 스냅샷을 불러온다. 실패하면 false.
	// 형식 검사는 상태를 바꾸기 전에 끝내지만, 시스템이 자기 상태를 읽다 실패하면
	// 앞선 시스템은 이미 바뀌었을 수 있다. 그래서 새로 만든 시뮬레이션에 불러오고,
	// 실패하면 그 시뮬레이션을 버린다.
	KINGDOMCORE_API bool Load(const vector<uint8>& Bytes);

	uint64 CurrentTick() const { return Clock.CurrentTick(); }
	uint64 GetSeed() const { return Seed; }

	FRandomStream& GetRandom() { return Random; }
	const FRandomStream& GetRandom() const { return Random; }

	// 처리된 커맨드 기록(받아들인 것과 거부한 것 모두, 마지막 세이브 이후)
	const vector<FCommandRecord>& GetCommandLog() const { return CommandLog; }
	size_t GetPendingCommandCount() const { return PendingCommands.size(); }

private:
	void TickOnce();
	void ProcessPendingCommands();

	shared_ptr<ICommand> CreateCommand(uint32 TypeId) const;
	void SaveCommand(FArchiveWriter& Writer, const ICommand& Command) const;
	shared_ptr<ICommand> LoadCommand(FArchiveReader& Reader) const;

	struct FPendingCommand
	{
		uint64 Sequence = 0;
		shared_ptr<ICommand> Command;
	};

	struct FCommandType
	{
		uint32 TypeId = 0;
		FCommandFactory Factory = nullptr;
	};

	uint64 Seed = 0;
	FSimulationClock Clock;
	FRandomStream Random;
	vector<unique_ptr<ISimulationSystem>> Systems;
	vector<FCommandType> CommandTypes;

	uint64 NextSequence = 0;
	vector<FPendingCommand> PendingCommands;
	vector<FCommandRecord> CommandLog;
};

} // namespace Kingdom
