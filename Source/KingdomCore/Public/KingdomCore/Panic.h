#pragma once

#include "KingdomCore/CoreStandard.h"

namespace Kingdom
{

// 더 진행할 수 없는 오류(데이터가 깨짐 등)에서 메시지를 남기고 프로세스를 즉시 끝낸다.
// 콘솔 도구와 장래의 서버가 쓴다.
// UE에서는 부르지 않는다: UE 쪽은 오류 목록을 받아 UE 방식으로 멈춘다(에디터는 플레이 중단).
[[noreturn]] KINGDOMCORE_API void Panic(const string& Message);

} // namespace Kingdom
