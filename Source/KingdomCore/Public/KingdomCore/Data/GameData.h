#pragma once

#include "KingdomCore/Data/CsvTable.h"
#include "KingdomCore/Data/TerrainDefinition.h"

namespace Kingdom
{

// 규칙 데이터 파일 이름. 저장소 루트의 Data/ 폴더에 있다.
// 코어는 파일을 열지 않는다: 바깥(UE, 테스트, 도구)이 이 이름으로 파일을 읽어 내용을 넘긴다.
namespace DataFiles
{
inline constexpr const char* Terrain = "Terrain.csv";
}

// 파일 내용(UTF-8 문자열) 묶음
struct FGameDataSources
{
	string TerrainCsv;
};

// 게임 규칙 데이터 전부. 로드한 뒤에는 바뀌지 않고, 시뮬레이션은 읽기만 한다.
// 세이브에 넣지 않는다(상태가 아니라 규칙). 세이브가 이 데이터를 가리킬 때는 문자열 Id로 쓴다.
class FGameData
{
public:
	const FTerrainDefinition& GetTerrain(ETerrainType Type) const { return Terrains[static_cast<size_t>(Type)]; }

	// 오류가 하나라도 있으면 OutData는 쓰지 않는다(일부만 읽힌 데이터로 게임이 돌지 않게).
	KINGDOMCORE_API static vector<FDataError> Load(const FGameDataSources& Sources, FGameData& OutData);

	// 콘솔 도구·장래 서버용. 오류가 있으면 전부 출력하고 Panic.
	// UE는 이것을 쓰지 않고 Load의 오류 목록을 받아 UE 방식으로 멈춘다.
	KINGDOMCORE_API static FGameData LoadOrPanic(const FGameDataSources& Sources);

private:
	array<FTerrainDefinition, TerrainTypeCount> Terrains{};
};

} // namespace Kingdom
