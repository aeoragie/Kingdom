#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "KingdomGameInstance.generated.h"

// 게임 전체의 수명을 가진 객체. 서브시스템(UKingdomSimulationSubsystem)은 이것에 붙어 산다.
UCLASS()
class KINGDOMGAME_API UKingdomGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// UFUNCTION(Exec): 콘솔(~ 키)에서 함수 이름으로 부를 수 있게 한다. 운영 툴의 관리자 명령 같은 것.
	// GameInstance는 엔진이 콘솔 명령을 찾아보는 곳 중 하나라 여기에 둔다(서브시스템은 그 대상이 아니다).

	// 예) KingdomSpeed 8
	UFUNCTION(Exec)
	void KingdomSpeed(int32 Multiplier);

	// 일시정지와 재개를 번갈아
	UFUNCTION(Exec)
	void KingdomPause();
};
