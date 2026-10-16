#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

#include "KingdomCore/Simulation.h"
#include "KingdomCore/TickPacer.h"

// UHT가 만드는 리플렉션 코드. 반드시 마지막 include여야 한다.
#include "KingdomSimulationSubsystem.generated.h"

// 게임이 켜져 있는 동안 코어 시뮬레이션 하나를 들고, 실제 시간에 맞춰 틱을 진행한다.
//
// UGameInstanceSubsystem: GameInstance가 만들어질 때 엔진이 같이 만들고, 끝날 때 같이 지운다.
//   서버로 치면 세션 수명의 서비스. 어디서든 GameInstance->GetSubsystem<>()으로 찾는다.
// FTickableGameObject: 상속하면 엔진의 매 프레임 갱신 목록에 들어간다.
//   서브시스템은 스스로 Tick을 받지 못하므로 함께 상속한다.
//
// 코어는 UE를 모른다. 이 클래스는 시간·배속·일시정지만 다루고 코어에는 "틱 N번"만 넘긴다.
UCLASS()
class KINGDOMGAME_API UKingdomSimulationSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	// UHT가 리플렉션·생성자 보조 코드를 끼워 넣는 자리. 클래스 본문 첫 줄.
	GENERATED_BODY()

public:
	UKingdomSimulationSubsystem();

	//~ Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem

	//~ Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject

	// 콘솔 명령(UKingdomGameInstance)이 부른다. 허용 범위 밖이면 무시하고 false.
	bool SetSpeed(int32 Multiplier);
	void TogglePause();

	uint64 GetCurrentTick() const;

	// 싱글의 틱 간격: 온라인 30초의 8배 빠르기(기획서 11. 개발과 기술 › 틱 간격은 설정값).
	// 설정 파일로 빼는 것은 데이터 파이프라인 때.
	static constexpr uint64 TickIntervalMicroseconds = 3750000;

	// 콘솔에서 받는 배속 상한. 화면 UI는 1·2·4·8만 쓰고, 64는 테스트용.
	static constexpr int32 MaxSpeed = 64;

private:
	// 일반 C++ 객체라 UPROPERTY를 붙이지 않는다(GC는 UObject만 추적한다).
	// TUniquePtr이 소유하므로 이 서브시스템이 지워질 때 같이 지워진다.
	TUniquePtr<Kingdom::FSimulation> Simulation;

	// 값 객체라 그대로 멤버로 둔다.
	// 기본값을 여기 써 두는 이유: UHT가 만드는 보조 생성자도 이 값으로 초기화되게 하려고.
	Kingdom::FTickPacer Pacer{TickIntervalMicroseconds};
};
