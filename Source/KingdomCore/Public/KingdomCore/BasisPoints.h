#pragma once

#include "KingdomCore/CoreTypes.h"

namespace Kingdom
{

// 비율은 만분율(basis points) 정수로 쓴다. 100 = 1%(0.01), 10000 = 100%(1.0).
// 데이터 열 이름과 변수 이름에는 BasisPoints를 붙여 퍼센트와 헷갈리지 않게 한다.
// 계산식에서 10000을 직접 쓰지 말고 이 상수를 쓴다.
inline constexpr int32 BasisPointsPerOne = 10000;

} // namespace Kingdom
