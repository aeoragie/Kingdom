#pragma once

#include "KingdomCore/Command.h"
#include "KingdomCore/CoreTypes.h"
#include "KingdomCore/RandomStream.h"
#include "KingdomCore/SimClock.h"

namespace Kingdom
{

class FSimulation;

// 틱마다 한 번 도는 규칙 단위(생산, 이동, 전투 …).
// 시스템은 등록한 순서대로 돈다. 순서도 결정론의 일부다.
class ISimSystem
{
public:
	virtual ~ISimSystem() = default;

	virtual const char* GetName() const = 0;
	virtual void Tick(FSimulation& Simulation) = 0;
};

// 고정 틱 시뮬레이션 루프.
// 코어는 "틱 N번 진행"만 안다. 틱 하나가 몇 초인지, 배속과 일시정지는 바깥(뷰)이 정한다.
//
// 한 틱의 순서:
//   1. 시계를 1 올린다(시스템과 커맨드는 CurrentTick()으로 지금 처리 중인 틱을 본다, 1부터)
//   2. 대기 중인 커맨드를 제출 순서대로 Validate → Apply
//   3. 시스템을 등록 순서대로 Tick
class FSimulation
{
public:
	explicit KINGDOMCORE_API FSimulation(uint64 Seed);
	KINGDOMCORE_API ~FSimulation();

	FSimulation(const FSimulation&) = delete;
	FSimulation& operator=(const FSimulation&) = delete;

	KINGDOMCORE_API void AddSystem(std::unique_ptr<ISimSystem> System);

	// 이름으로 시스템을 찾는다. 없으면 nullptr. 커맨드가 자기 대상 시스템을 찾을 때 쓴다.
	KINGDOMCORE_API ISimSystem* FindSystem(const char* Name);
	KINGDOMCORE_API const ISimSystem* FindSystem(const char* Name) const;

	// 커맨드를 제출한다. 다음 틱 처음에 처리된다. 돌려주는 값은 제출 순번.
	KINGDOMCORE_API uint64 SubmitCommand(std::unique_ptr<ICommand> Command);

	// TickCount만큼 틱을 진행한다.
	KINGDOMCORE_API void Advance(uint32 TickCount);

	uint64 CurrentTick() const { return Clock.CurrentTick(); }
	uint64 GetSeed() const { return Seed; }

	FRandomStream& GetRandom() { return Random; }
	const FRandomStream& GetRandom() const { return Random; }

	// 처리된 커맨드 기록(받아들인 것과 거부한 것 모두)
	const std::vector<FCommandRecord>& GetCommandLog() const { return CommandLog; }
	std::size_t GetPendingCommandCount() const { return PendingCommands.size(); }

private:
	void TickOnce();
	void ProcessPendingCommands();

	struct FPendingCommand
	{
		uint64 Sequence = 0;
		std::shared_ptr<ICommand> Command;
	};

	uint64 Seed = 0;
	FSimClock Clock;
	FRandomStream Random;
	std::vector<std::unique_ptr<ISimSystem>> Systems;

	uint64 NextSequence = 0;
	std::vector<FPendingCommand> PendingCommands;
	std::vector<FCommandRecord> CommandLog;
};

} // namespace Kingdom
