#pragma once

#include "KingdomCore/Panic.h"

// 코드 버그(범위를 넘는 인덱스, 있을 수 없는 enum 값 등)를 그 자리에서 잡는다.
// 실패하면 조건식·파일·줄을 담아 Panic한다. 디버거가 붙어 있으면 그 자리에서 멈춘다.
//
// UE의 check()와 달리 릴리스 빌드에서도 켜져 있다.
// 인덱스 비교 한 번은 비용이 거의 없고, 릴리스에서 정의되지 않은 동작이 나는 쪽이 더 위험하다.
//
// 바깥 입력(데이터 파일, 세이브)이 틀린 것은 버그가 아니므로 이것을 쓰지 않는다.
// 그쪽은 실패나 오류 목록을 돌려준다.
#define KINGDOM_CHECK(Condition) \
	((Condition) ? static_cast<void>(0) : ::Kingdom::Detail::CheckFailed(#Condition, __FILE__, __LINE__))

namespace Kingdom::Detail
{

[[noreturn]] KINGDOMCORE_API void CheckFailed(const char* Condition, const char* File, int Line);

} // namespace Kingdom::Detail
