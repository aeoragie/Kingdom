#pragma once

// KingdomCore의 최상위 표준 라이브러리 헤더. 코어가 쓰는 std 헤더는 전부 여기에만
// 모은다 — 개별 파일에서 <cstdint> 같은 걸 직접 포함하지 않는다.
// UE 헤더는 절대 넣지 않는다 (CoreMinimal.h 한 줄이면 코어 분리가 끝난다).

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// 심볼 export 매크로. UE 모듈 빌드에서는 UBT가 KINGDOMCORE_API를
// DLLEXPORT/DLLIMPORT로 정의해 주고, CMake 독립 빌드(정적 라이브러리)에서는 빈 값이다.
// DLLEXPORT/DLLIMPORT는 원래 UE 플랫폼 헤더가 정의하는데, 코어는 UE 헤더를 넣지 않으므로
// 없을 때만 같은 뜻으로 정의한다. 선언 순서는 `explicit KINGDOMCORE_API F…(…)`처럼
// 매크로를 explicit·virtual 뒤에 둔다.
#if defined(_MSC_VER)
	#ifndef DLLEXPORT
		#define DLLEXPORT __declspec(dllexport)
	#endif
	#ifndef DLLIMPORT
		#define DLLIMPORT __declspec(dllimport)
	#endif
#else
	#ifndef DLLEXPORT
		#define DLLEXPORT __attribute__((visibility("default")))
	#endif
	#ifndef DLLIMPORT
		#define DLLIMPORT __attribute__((visibility("default")))
	#endif
#endif

#ifndef KINGDOMCORE_API
	#define KINGDOMCORE_API
#endif

// STL 컨테이너와 스마트 포인터는 Kingdom 네임스페이스 안에서 std:: 없이 쓴다.
// 전역 using namespace std는 쓰지 않는다. 코어 헤더는 UE 모듈에서도 include되므로
// 전역에 풀면 UE·Windows 헤더의 이름(byte, min, max 등)과 충돌한다.
// 함수(std::move, std::strcmp 등)는 ADL 문제를 피하려고 std::를 그대로 붙인다.
namespace Kingdom
{

using std::array;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

} // namespace Kingdom
