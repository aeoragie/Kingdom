#include "MiniTest.h"

#include "KingdomCore/BasisPoints.h"
#include "KingdomCore/Data/CsvTable.h"
#include "KingdomCore/Data/GameData.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// CMakeLists.txt가 저장소의 Data/ 폴더 경로를 넘겨준다
#ifndef KINGDOM_DATA_DIRECTORY
#error KINGDOM_DATA_DIRECTORY is not defined
#endif

namespace
{

std::string ReadDataFile(const char* FileName)
{
	std::ifstream File(std::string(KINGDOM_DATA_DIRECTORY) + "/" + FileName, std::ios::binary);
	std::ostringstream Stream;
	Stream << File.rdbuf();
	return Stream.str();
}

const char* const TerrainHeader = "Id,Name,MoveCost,DefenseBonusBasisPoints\n";

std::string AllTerrainRows()
{
	return "Plains,P,1,0\n"
		   "Hills,H,2,1000\n"
		   "Mountains,M,3,3000\n"
		   "River,R,2,2000\n"
		   "Forest,F,2,1000\n"
		   "Marsh,W,3,0\n";
}

bool HasError(const std::vector<Kingdom::FDataError>& Errors, Kingdom::uint32 Line, const char* Column, const char* MessagePart)
{
	for (const Kingdom::FDataError& Error : Errors)
	{
		if (Error.Line == Line && Error.Column == Column && Error.Message.find(MessagePart) != std::string::npos)
		{
			return true;
		}
	}
	return false;
}

} // namespace

// ---------------------------------------------------------------------------
// FCsvTable

TEST_CASE(Csv_ParsesHeaderAndRowsByColumnName)
{
	Kingdom::FCsvTable Table;
	std::vector<Kingdom::FDataError> Errors;
	CHECK(Kingdom::FCsvTable::Parse("Id,Value\nA,1\nB,2\n", "Test.csv", Table, Errors));
	CHECK(Errors.empty());
	CHECK(Table.GetRowCount() == 2);

	size_t ValueColumn = 0;
	CHECK(Table.FindColumn("Value", ValueColumn));
	CHECK(ValueColumn == 1);
	CHECK(Table.GetCell(1, ValueColumn) == "2");
	CHECK(Table.GetLine(1) == 3);

	size_t Missing = 0;
	CHECK(!Table.FindColumn("Nope", Missing));
}

TEST_CASE(Csv_QuotedFieldKeepsCommaNewlineAndQuote)
{
	Kingdom::FCsvTable Table;
	std::vector<Kingdom::FDataError> Errors;
	CHECK(Kingdom::FCsvTable::Parse("Id,Text\nA,\"x, \"\"y\"\"\nz\"\nB,end\n", "Test.csv", Table, Errors));
	CHECK(Table.GetRowCount() == 2);
	CHECK(Table.GetCell(0, 1) == "x, \"y\"\nz");
	CHECK(Table.GetLine(1) == 4); // 따옴표 안 줄바꿈도 줄 번호에 센다
}

TEST_CASE(Csv_HandlesBomCrlfAndEmptyLines)
{
	Kingdom::FCsvTable Table;
	std::vector<Kingdom::FDataError> Errors;
	CHECK(Kingdom::FCsvTable::Parse("\xEF\xBB\xBFId,Value\r\n\r\nA,1\r\nB,2", "Test.csv", Table, Errors));
	CHECK(Table.GetRowCount() == 2);

	size_t IdColumn = 99;
	CHECK(Table.FindColumn("Id", IdColumn)); // BOM이 첫 열 이름에 붙지 않는다
	CHECK(IdColumn == 0);
	CHECK(Table.GetCell(0, 1) == "1");       // \r이 칸에 남지 않는다
	CHECK(Table.GetCell(1, 1) == "2");       // 마지막 줄바꿈이 없어도 읽는다
}

TEST_CASE(Csv_RejectsNonUtf8)
{
	// CP949로 저장한 "한"
	Kingdom::FCsvTable Table;
	std::vector<Kingdom::FDataError> Errors;
	CHECK(!Kingdom::FCsvTable::Parse("Id,Name\nA,\xC7\xD1\n", "Test.csv", Table, Errors));
	CHECK(HasError(Errors, 0, "", "UTF-8"));
}

TEST_CASE(Csv_RejectsFieldCountMismatchWithLine)
{
	Kingdom::FCsvTable Table;
	std::vector<Kingdom::FDataError> Errors;
	CHECK(!Kingdom::FCsvTable::Parse("Id,Value\nA,1\nB\n", "Test.csv", Table, Errors));
	CHECK(HasError(Errors, 3, "", "fields"));
}

TEST_CASE(Csv_RejectsUnclosedQuoteAndDuplicateColumn)
{
	Kingdom::FCsvTable Table;
	std::vector<Kingdom::FDataError> Errors;
	CHECK(!Kingdom::FCsvTable::Parse("Id,Text\nA,\"open\n", "Test.csv", Table, Errors));
	CHECK(HasError(Errors, 2, "", "unclosed"));

	Errors.clear();
	CHECK(!Kingdom::FCsvTable::Parse("Id,Id\nA,B\n", "Test.csv", Table, Errors));
	CHECK(HasError(Errors, 1, "", "duplicate column"));
}

TEST_CASE(Csv_TryParseInt32AcceptsOnlyPlainIntegers)
{
	Kingdom::int32 Value = 0;
	CHECK(Kingdom::TryParseInt32("12", Value) && Value == 12);
	CHECK(Kingdom::TryParseInt32("-5", Value) && Value == -5);
	CHECK(!Kingdom::TryParseInt32("", Value));
	CHECK(!Kingdom::TryParseInt32(" 1", Value));
	CHECK(!Kingdom::TryParseInt32("1.5", Value));
	CHECK(!Kingdom::TryParseInt32("abc", Value));
	CHECK(!Kingdom::TryParseInt32("99999999999", Value));
}

// ---------------------------------------------------------------------------
// EnumToString / StringToEnum

TEST_CASE(EnumString_RoundTripsEveryTerrain)
{
	CHECK(Kingdom::TerrainTypeCount == 6);
	for (size_t Index = 0; Index < Kingdom::TerrainTypeCount; ++Index)
	{
		const Kingdom::ETerrainType Type = static_cast<Kingdom::ETerrainType>(Index);
		Kingdom::ETerrainType Parsed = Kingdom::ETerrainType::Plains;
		CHECK(Kingdom::StringToEnum(Kingdom::EnumToString(Type), Parsed));
		CHECK(Parsed == Type);
	}

	Kingdom::ETerrainType Parsed = Kingdom::ETerrainType::Hills;
	CHECK(!Kingdom::StringToEnum("plains", Parsed)); // 대소문자를 구별한다
	CHECK(Parsed == Kingdom::ETerrainType::Hills);   // 실패하면 바꾸지 않는다
	CHECK(Kingdom::EnumToString(Kingdom::ETerrainType::Mountains) == std::string("Mountains"));
}

// ---------------------------------------------------------------------------
// FGameData

TEST_CASE(GameData_LoadsRealTerrainFile)
{
	Kingdom::FGameDataSources Sources;
	Sources.TerrainCsv = ReadDataFile(Kingdom::DataFiles::Terrain);
	CHECK(!Sources.TerrainCsv.empty());

	Kingdom::FGameData Data;
	const std::vector<Kingdom::FDataError> Errors = Kingdom::FGameData::Load(Sources, Data);
	for (const Kingdom::FDataError& Error : Errors)
	{
		std::printf("  %s\n", Error.ToString().c_str());
	}
	CHECK(Errors.empty());

	for (size_t Index = 0; Index < Kingdom::TerrainTypeCount; ++Index)
	{
		const Kingdom::ETerrainType Type = static_cast<Kingdom::ETerrainType>(Index);
		const Kingdom::FTerrainDefinition& Terrain = Data.GetTerrain(Type);
		CHECK(Terrain.Type == Type);
		CHECK(!Terrain.Name.empty());
		CHECK(Terrain.MoveCost > 0);
		CHECK(Terrain.DefenseBonusBasisPoints >= 0 && Terrain.DefenseBonusBasisPoints <= Kingdom::BasisPointsPerOne);
	}
}

TEST_CASE(GameData_ColumnOrderDoesNotMatter)
{
	Kingdom::FGameDataSources Sources;
	Sources.TerrainCsv = "DefenseBonusBasisPoints,MoveCost,Name,Id\n"
						 "0,1,P,Plains\n1000,2,H,Hills\n3000,3,M,Mountains\n"
						 "2000,2,R,River\n1000,2,F,Forest\n0,3,W,Marsh\n";
	Kingdom::FGameData Data;
	CHECK(Kingdom::FGameData::Load(Sources, Data).empty());
	CHECK(Data.GetTerrain(Kingdom::ETerrainType::Mountains).DefenseBonusBasisPoints == 3000);
}

TEST_CASE(GameData_ReportsMissingColumn)
{
	Kingdom::FGameDataSources Sources;
	Sources.TerrainCsv = "Id,Name,MoveCost\nPlains,P,1\n";
	Kingdom::FGameData Data;
	const std::vector<Kingdom::FDataError> Errors = Kingdom::FGameData::Load(Sources, Data);
	CHECK(HasError(Errors, 0, "DefenseBonusBasisPoints", "missing column"));
}

TEST_CASE(GameData_ReportsBadNumberWithLineAndColumn)
{
	Kingdom::FGameDataSources Sources;
	Sources.TerrainCsv = std::string(TerrainHeader) + "Plains,P,1,0\nHills,H,2.5,1000\n"
						 "Mountains,M,3,3000\nRiver,R,2,2000\nForest,F,2,1000\nMarsh,W,3,0\n";
	Kingdom::FGameData Data;
	const std::vector<Kingdom::FDataError> Errors = Kingdom::FGameData::Load(Sources, Data);
	CHECK(Errors.size() == 1);
	CHECK(HasError(Errors, 3, "MoveCost", "not an integer"));
	CHECK(Errors[0].ToString() == "Terrain.csv:3 [MoveCost] not an integer: '2.5'");
}

TEST_CASE(GameData_ReportsUnknownDuplicateAndMissingTerrain)
{
	Kingdom::FGameDataSources Sources;
	Sources.TerrainCsv = std::string(TerrainHeader) + "Plains,P,1,0\nPlains,P,1,0\nVolcano,V,9,0\n";
	Kingdom::FGameData Data;
	const std::vector<Kingdom::FDataError> Errors = Kingdom::FGameData::Load(Sources, Data);
	CHECK(HasError(Errors, 3, "Id", "duplicate terrain 'Plains'"));
	CHECK(HasError(Errors, 4, "Id", "unknown terrain 'Volcano'"));
	CHECK(HasError(Errors, 0, "", "missing terrain 'Hills'"));
	CHECK(HasError(Errors, 0, "", "missing terrain 'Marsh'"));
}

TEST_CASE(GameData_FailedLoadLeavesOutputUntouched)
{
	Kingdom::FGameDataSources Good;
	Good.TerrainCsv = std::string(TerrainHeader) + AllTerrainRows();
	Kingdom::FGameData Data;
	CHECK(Kingdom::FGameData::Load(Good, Data).empty());

	Kingdom::FGameDataSources Bad;
	Bad.TerrainCsv = std::string(TerrainHeader) + "Plains,Changed,99,0\n";
	CHECK(!Kingdom::FGameData::Load(Bad, Data).empty());
	CHECK(Data.GetTerrain(Kingdom::ETerrainType::Plains).MoveCost == 1);
	CHECK(Data.GetTerrain(Kingdom::ETerrainType::Plains).Name == "P");
}
