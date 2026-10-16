#pragma once

#include "KingdomCore/EnumString.h"

namespace Kingdom
{

// 지형 6종(기획서 3. 땅 › 지형). 종류가 많아지면 플레이어가 지도를 못 읽으므로 6종으로 제한한다.
#define KINGDOM_TERRAIN_TYPES(X) \
	X(Plains)    /* 평지 */ \
	X(Hills)     /* 구릉 */ \
	X(Mountains) /* 산악 */ \
	X(River)     /* 강 */ \
	X(Forest)    /* 숲 */ \
	X(Marsh)     /* 습지 */

KINGDOM_DEFINE_ENUM(ETerrainType, TerrainTypeCount, KINGDOM_TERRAIN_TYPES)

// Data/Terrain.csv 한 행. 이동 비용과 방어 보너스를 같은 표에서 읽는다(기획서 11 › 필드 › 지형 데이터).
struct FTerrainDefinition
{
	ETerrainType Type = ETerrainType::Plains;
	string Name;                         // 표시 이름(디버그·콘솔용). 화면 글자는 나중에 UE 쪽 표현 데이터로
	int32 MoveCost = 0;                  // 헥스 하나를 지나는 비용
	int32 DefenseBonusBasisPoints = 0;   // 만분율. 3000 = +30%
};

} // namespace Kingdom
