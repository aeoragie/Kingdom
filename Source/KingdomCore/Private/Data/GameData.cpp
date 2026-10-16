#include "KingdomCore/Data/GameData.h"

#include "KingdomCore/Panic.h"

namespace Kingdom
{

namespace
{

// 표 하나를 읽을 때 쓰는 도우미. 열을 이름으로 찾고, 칸을 타입별로 읽으며 오류를 모은다.
class FTableReader
{
public:
	FTableReader(const FCsvTable& InTable, vector<FDataError>& InErrors)
		: Table(InTable)
		, Errors(InErrors)
	{
	}

	// 필요한 열이 없으면 오류. 모든 열을 먼저 찾은 뒤 하나라도 없으면 행을 읽지 않는다.
	bool RequireColumn(const char* ColumnName, size_t& OutColumn)
	{
		if (Table.FindColumn(ColumnName, OutColumn))
		{
			return true;
		}
		Errors.push_back({Table.GetSourceName(), 0, ColumnName, "missing column"});
		return false;
	}

	const string& Text(size_t Row, size_t Column) const
	{
		return Table.GetCell(Row, Column);
	}

	bool Int32(size_t Row, size_t Column, const char* ColumnName, int32& OutValue)
	{
		if (TryParseInt32(Table.GetCell(Row, Column), OutValue))
		{
			return true;
		}
		AddError(Row, ColumnName, "not an integer: '" + Table.GetCell(Row, Column) + "'");
		return false;
	}

	void AddError(size_t Row, const char* ColumnName, const string& Message)
	{
		Errors.push_back({Table.GetSourceName(), Table.GetLine(Row), ColumnName, Message});
	}

	void AddFileError(const string& Message)
	{
		Errors.push_back({Table.GetSourceName(), 0, string(), Message});
	}

private:
	const FCsvTable& Table;
	vector<FDataError>& Errors;
};

void LoadTerrains(const string& Text, array<FTerrainDefinition, TerrainTypeCount>& OutTerrains, vector<FDataError>& Errors)
{
	FCsvTable Table;
	if (!FCsvTable::Parse(Text, DataFiles::Terrain, Table, Errors))
	{
		return;
	}

	FTableReader Reader(Table, Errors);
	size_t IdColumn = 0;
	size_t NameColumn = 0;
	size_t MoveCostColumn = 0;
	size_t DefenseColumn = 0;
	bool bColumnsFound = Reader.RequireColumn("Id", IdColumn);
	bColumnsFound &= Reader.RequireColumn("Name", NameColumn);
	bColumnsFound &= Reader.RequireColumn("MoveCost", MoveCostColumn);
	bColumnsFound &= Reader.RequireColumn("DefenseBonusBasisPoints", DefenseColumn);
	if (!bColumnsFound)
	{
		return;
	}

	// 지형 종류는 코드가 정한다(enum). 표는 종류마다 정확히 한 줄이어야 한다.
	array<bool, TerrainTypeCount> bSeen{};
	for (size_t Row = 0; Row < Table.GetRowCount(); ++Row)
	{
		ETerrainType Type = ETerrainType::Plains;
		if (!StringToEnum(Reader.Text(Row, IdColumn), Type))
		{
			Reader.AddError(Row, "Id", "unknown terrain '" + Reader.Text(Row, IdColumn) + "'");
			continue;
		}

		const size_t Index = static_cast<size_t>(Type);
		if (bSeen[Index])
		{
			Reader.AddError(Row, "Id", "duplicate terrain '" + Reader.Text(Row, IdColumn) + "'");
			continue;
		}
		bSeen[Index] = true;

		FTerrainDefinition& Terrain = OutTerrains[Index];
		Terrain.Type = Type;
		Terrain.Name = Reader.Text(Row, NameColumn);
		Reader.Int32(Row, MoveCostColumn, "MoveCost", Terrain.MoveCost);
		Reader.Int32(Row, DefenseColumn, "DefenseBonusBasisPoints", Terrain.DefenseBonusBasisPoints);
	}

	for (size_t Index = 0; Index < TerrainTypeCount; ++Index)
	{
		if (!bSeen[Index])
		{
			Reader.AddFileError(string("missing terrain '") + EnumToString(static_cast<ETerrainType>(Index)) + "'");
		}
	}
}

} // namespace

vector<FDataError> FGameData::Load(const FGameDataSources& Sources, FGameData& OutData)
{
	vector<FDataError> Errors;
	FGameData Loaded;

	LoadTerrains(Sources.TerrainCsv, Loaded.Terrains, Errors);

	if (Errors.empty())
	{
		OutData = std::move(Loaded);
	}
	return Errors;
}

FGameData FGameData::LoadOrPanic(const FGameDataSources& Sources)
{
	FGameData Data;
	const vector<FDataError> Errors = Load(Sources, Data);
	if (!Errors.empty())
	{
		string Message = std::to_string(Errors.size()) + " data error(s)";
		for (const FDataError& Error : Errors)
		{
			Message += "\n  " + Error.ToString();
		}
		Panic(Message);
	}
	return Data;
}

} // namespace Kingdom
