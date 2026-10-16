#include "KingdomGameInstance.h"

#include "KingdomSimulationSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogKingdomConsole, Log, All);

void UKingdomGameInstance::KingdomSpeed(int32 Multiplier)
{
	// GetSubsystem<>(): 이 GameInstance에 붙은 서브시스템을 타입으로 찾는다. 서비스 로케이터 조회와 같다.
	UKingdomSimulationSubsystem* SimulationSubsystem = GetSubsystem<UKingdomSimulationSubsystem>();
	if (!SimulationSubsystem)
	{
		return;
	}

	if (!SimulationSubsystem->SetSpeed(Multiplier))
	{
		UE_LOG(LogKingdomConsole, Warning, TEXT("KingdomSpeed: 1 ~ %d"), UKingdomSimulationSubsystem::MaxSpeed);
	}
}

void UKingdomGameInstance::KingdomPause()
{
	if (UKingdomSimulationSubsystem* SimulationSubsystem = GetSubsystem<UKingdomSimulationSubsystem>())
	{
		SimulationSubsystem->TogglePause();
	}
}
