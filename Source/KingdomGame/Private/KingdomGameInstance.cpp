#include "KingdomGameInstance.h"

#include "Engine/Engine.h"

#include "KingdomCore/SimClock.h"
#include "KingdomCore/Version.h"

void UKingdomGameInstance::Init()
{
	Super::Init();

	Kingdom::FSimClock Clock;
	Clock.Advance(10);

	const FString Message = FString::Printf(
		TEXT("KingdomCore %hs | SimClock tick=%llu"),
		Kingdom::CoreVersion(),
		static_cast<unsigned long long>(Clock.CurrentTick()));

	UE_LOG(LogTemp, Log, TEXT("%s"), *Message);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Yellow, Message);
	}
}
