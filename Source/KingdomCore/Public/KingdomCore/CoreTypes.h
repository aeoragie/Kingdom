#pragma once

// UE와 같은 표기의 기본 타입 별칭. KingdomCore는 UE 헤더 금지이므로 표준 타입에
// 별칭만 씌운다. Kingdom 네임스페이스 안에 두어 UE 전역 별칭(int32 등)과 충돌하지 않는다.

#include "KingdomCore/CoreStandard.h"

namespace Kingdom
{

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

} // namespace Kingdom
