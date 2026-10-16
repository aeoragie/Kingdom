#pragma once

// 코어의 정수 타입은 전부 여기서 정한다.
// UE와 같은 표기의 별칭(int32, uint64 등)을 쓰고, 표준 이름(uint64_t 등)은 코드에서 쓰지 않는다.
// KingdomCore는 UE 헤더 금지이므로 표준 타입에 별칭만 씌운다.
// Kingdom 네임스페이스 안에 두어 UE 전역 별칭(int32 등)과 충돌하지 않는다.
// UE식 별칭이 없는 size_t·ptrdiff_t는 표준 이름 그대로 들여온다.

#include "KingdomCore/CoreStandard.h"

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

using std::size_t;
using std::ptrdiff_t;

} // namespace Kingdom
