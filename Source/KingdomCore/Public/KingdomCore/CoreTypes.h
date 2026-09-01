#pragma once

// UE와 같은 표기의 기본 타입 별칭. KingdomCore는 UE 헤더 금지이므로 표준 타입에
// 별칭만 씌운다. Kingdom 네임스페이스 안에 두어 UE 전역 별칭(int32 등)과 충돌하지 않는다.

#include "KingdomCore/CoreStd.h"

namespace Kingdom
{

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

} // namespace Kingdom
