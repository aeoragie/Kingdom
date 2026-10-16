#include "KingdomSimulationSubsystem.h"

#include "Engine/Engine.h"

namespace
{

// 새 게임·세이브 연결은 뒤 단계에서. 지금은 고정 시드.
constexpr uint64 DefaultSeed = 1;

// AddOnScreenDebugMessage에 고정 키를 주면 매 프레임 새 줄을 쌓지 않고 같은 줄을 덮어쓴다.
constexpr uint64 StatusMessageKey = 0x4B494E47; // "KING"

} // namespace

// UObject에 FTickableGameObject를 붙일 때의 엔진 규칙: Never로 만들고, 준비가 끝나면 켠다.
// UObject는 클래스마다 기본값 견본(CDO)도 만들어지는데, 그것까지 틱하면 안 되기 때문이다.
// 엔진의 UTickableWorldSubsystem도 같은 방식이다.
UKingdomSimulationSubsystem::UKingdomSimulationSubsystem()
	: FTickableGameObject(ETickableTickType::Never)
{
}

void UKingdomSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Simulation = MakeUnique<Kingdom::FSimulation>(DefaultSeed);

	// 준비가 끝났으니 틱을 켠다. 켜도 되는지는 GetTickableTickType이 판단한다.
	SetTickableTickType(GetTickableTickType());
}

void UKingdomSimulationSubsystem::Deinitialize()
{
	// 지우기 전에 틱부터 끈다. 반쯤 지워진 상태에서 Tick이 불리지 않게.
	SetTickableTickType(ETickableTickType::Never);
	Simulation.Reset();

	Super::Deinitialize();
}

ETickableTickType UKingdomSimulationSubsystem::GetTickableTickType() const
{
	// IsTemplate(): CDO(클래스 기본값 견본)인지. 견본과 아직 준비 안 된 객체는 틱하지 않는다.
	if (IsTemplate() || !Simulation)
	{
		return ETickableTickType::Never;
	}
	return ETickableTickType::Always;
}

TStatId UKingdomSimulationSubsystem::GetStatId() const
{
	// 프로파일러(stat 명령, Unreal Insights)에서 이 Tick이 쓴 시간을 묶어 보는 이름표.
	RETURN_QUICK_DECLARE_CYCLE_STAT(UKingdomSimulationSubsystem, STATGROUP_Tickables);
}

void UKingdomSimulationSubsystem::Tick(float DeltaTime)
{
	// 실제 시간은 정수 마이크로초로 넘긴다. 몇 틱을 돌릴지는 코어의 FTickPacer가 정한다.
	// (일시정지, 배속, 멈춰 있던 시간의 상한도 FTickPacer가 처리한다)
	const uint64 ElapsedMicroseconds = static_cast<uint64>(DeltaTime * 1000000.0);
	const uint32 TickCount = Pacer.Consume(ElapsedMicroseconds);
	if (TickCount > 0)
	{
		Simulation->Advance(TickCount);
	}

	if (GEngine)
	{
		// 기본 디버그 글꼴에 한글이 없어 영문으로 띄운다.
		const FString Status = FString::Printf(
			TEXT("Tick %llu | x%u%s"),
			static_cast<unsigned long long>(Simulation->CurrentTick()),
			Pacer.GetSpeed(),
			Pacer.IsPaused() ? TEXT(" | PAUSED") : TEXT(""));

		// 표시 시간 0초: 이번 프레임만. 매 프레임 다시 그리므로 계속 보인다.
		GEngine->AddOnScreenDebugMessage(StatusMessageKey, 0.0f, FColor::Yellow, Status);
	}
}

bool UKingdomSimulationSubsystem::SetSpeed(int32 Multiplier)
{
	if (Multiplier < 1 || Multiplier > MaxSpeed)
	{
		return false;
	}

	Pacer.SetSpeed(static_cast<uint32>(Multiplier));
	return true;
}

void UKingdomSimulationSubsystem::TogglePause()
{
	Pacer.SetPaused(!Pacer.IsPaused());
}

uint64 UKingdomSimulationSubsystem::GetCurrentTick() const
{
	return Simulation ? Simulation->CurrentTick() : 0;
}
