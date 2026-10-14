#pragma once

#include "KingdomCore/CoreTypes.h"

namespace Kingdom
{

class FArchiveReader;
class FArchiveWriter;
class FSimulation;

// 검증 결과. 거부할 때는 이유를 함께 돌려준다(로그와 화면 알림에 쓴다).
struct FCommandValidation
{
	bool bValid = true;
	const char* Reason = "";

	static FCommandValidation Ok() { return {true, ""}; }
	static FCommandValidation Fail(const char* InReason) { return {false, InReason}; }
};

// 상태를 바꾸는 유일한 길. UE 쪽은 코어 상태를 직접 고치지 않고 커맨드만 제출한다.
// 제출된 커맨드는 다음 틱 처음에 제출 순서대로 Validate → Apply 된다.
// 검증을 적용 시점에 하는 이유: 제출과 적용 사이에 상태가 바뀔 수 있기 때문이다.
//
// 세이브에 담기 위해 커맨드는 자기 종류 번호(TypeId)와 내용을 직렬화할 줄 안다.
// 불러올 때는 FSimulation에 등록된 종류 번호 → 생성 함수 표로 다시 만든다
// (서버에서 패킷 ID로 메시지를 만드는 것과 같은 방식).
class ICommand
{
public:
	virtual ~ICommand() = default;

	virtual uint32 GetTypeId() const = 0;
	virtual const char* GetName() const = 0;
	virtual FCommandValidation Validate(const FSimulation& Simulation) const = 0;
	virtual void Apply(FSimulation& Simulation) = 0;

	virtual void Save(FArchiveWriter& Writer) const = 0;
	virtual bool Load(FArchiveReader& Reader) = 0;
};

using FCommandFactory = unique_ptr<ICommand> (*)();

// 커맨드 로그 한 줄. 받아들인 것과 거부한 것을 모두 남긴다.
// 세이브는 전체 상태 스냅샷이고, 이 로그는 마지막 세이브 이후 것만 함께 저장한다.
struct FCommandRecord
{
	uint64 Sequence = 0;	// 제출 순번(0부터)
	uint64 AppliedTick = 0;	// 처리된 틱
	bool bAccepted = false;
	string Reason;
	shared_ptr<ICommand> Command;
};

} // namespace Kingdom
