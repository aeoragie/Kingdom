#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "KingdomGameInstance.generated.h"

// Phase 0 게이트: 코어 함수 호출값을 화면에 찍는다.
UCLASS()
class KINGDOMGAME_API UKingdomGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
};
