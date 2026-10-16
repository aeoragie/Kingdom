#pragma once

#include "KingdomCore/Check.h"
#include "KingdomCore/CoreTypes.h"

// 코드가 종류를 못 박은 enum(지형, 자원, 병종 계열 등)을 정의한다.
// 파일(CSV, 세이브)에는 문자열로, 메모리에는 enum으로 둔다.
//
// 목록 매크로 하나에서 enum, 개수, EnumToString, StringToEnum이 함께 만들어진다.
// 종류를 더할 때 목록에 한 줄만 고치면 되고, 이름 표와 enum이 어긋날 수 없다.
//
//   #define KINGDOM_TERRAIN_TYPES(X) X(Plains) X(Hills)
//   KINGDOM_DEFINE_ENUM(ETerrainType, TerrainTypeCount, KINGDOM_TERRAIN_TYPES)
//
//   EnumToString(ETerrainType::Hills)       -> "Hills"
//   StringToEnum("Hills", Type)             -> true, Type = ETerrainType::Hills
//   for (size_t Index = 0; Index < TerrainTypeCount; ++Index) ...

#define KINGDOM_ENUM_VALUE(Name) Name,
#define KINGDOM_ENUM_NAME(Name) #Name,

#define KINGDOM_DEFINE_ENUM(EnumName, CountName, List) \
	enum class EnumName : uint8 \
	{ \
		List(KINGDOM_ENUM_VALUE) \
	}; \
	\
	inline constexpr const char* EnumName##Names[] = { List(KINGDOM_ENUM_NAME) }; \
	inline constexpr size_t CountName = sizeof(EnumName##Names) / sizeof(EnumName##Names[0]); \
	\
	inline const char* EnumToString(EnumName Value) \
	{ \
		const size_t Index = static_cast<size_t>(Value); \
		KINGDOM_CHECK(Index < CountName); \
		return EnumName##Names[Index]; \
	} \
	\
	inline bool StringToEnum(const string& Text, EnumName& OutValue) \
	{ \
		for (size_t Index = 0; Index < CountName; ++Index) \
		{ \
			if (Text == EnumName##Names[Index]) \
			{ \
				OutValue = static_cast<EnumName>(Index); \
				return true; \
			} \
		} \
		return false; \
	}
