#pragma once

// KingdomCore의 최상위 표준 라이브러리 헤더. 코어가 쓰는 std 헤더는 전부 여기에만
// 모은다 — 개별 파일에서 <cstdint> 같은 걸 직접 포함하지 않는다.
// UE 헤더는 절대 넣지 않는다 (CoreMinimal.h 한 줄이면 코어 분리가 끝난다).

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

// 심볼 export 매크로. UE 모듈 빌드에서는 UBT가 KINGDOMCORE_API를
// dllexport/dllimport로 정의해 주고, CMake 독립 빌드(정적 라이브러리)에서는 빈 값이다.
#ifndef KINGDOMCORE_API
	#define KINGDOMCORE_API
#endif
