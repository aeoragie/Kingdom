#pragma once

#include "KingdomCore/CoreTypes.h"

namespace Kingdom
{

// 데이터를 읽다 난 오류 하나. 어느 파일·몇 번째 줄·어느 열인지 함께 남긴다.
struct FDataError
{
	string Source;
	uint32 Line = 0; // 파일의 줄 번호(1부터). 파일 전체 오류면 0
	string Column;   // 열 이름. 해당 없으면 빈 문자열
	string Message;

	KINGDOMCORE_API string ToString() const;
};

// CSV 문자열 하나를 행·열로 나눈 것.
// 첫 줄은 열 이름(헤더)이다. 열은 이름으로 찾는다(열 순서를 바꿔도 된다).
//
// 처리하는 것: UTF-8 BOM, CRLF/LF, 따옴표로 감싼 칸(안의 쉼표·줄바꿈, ""는 따옴표 하나), 빈 줄.
// UTF-8이 아닌 파일(엑셀의 그냥 "CSV" 저장 = CP949)은 오류로 거절한다.
class FCsvTable
{
public:
	// 실패하면 false이고 OutErrors에 이유를 더한다.
	KINGDOMCORE_API static bool Parse(const string& Text, const string& SourceName, FCsvTable& OutTable, vector<FDataError>& OutErrors);

	const string& GetSourceName() const { return SourceName; }
	size_t GetRowCount() const { return Rows.size(); }

	// 열 이름으로 열 번호를 찾는다. 없으면 false.
	KINGDOMCORE_API bool FindColumn(const string& ColumnName, size_t& OutColumn) const;

	const string& GetCell(size_t Row, size_t Column) const { return Rows[Row][Column]; }

	// 그 행이 파일의 몇 번째 줄에서 시작하는지(오류 메시지용)
	uint32 GetLine(size_t Row) const { return RowLines[Row]; }

private:
	string SourceName;
	vector<string> Header;
	vector<vector<string>> Rows;
	vector<uint32> RowLines;
};

// 정수 칸을 읽는다. 앞뒤 공백, 소수점, 숫자 아닌 글자, 범위 초과는 실패.
KINGDOMCORE_API bool TryParseInt32(const string& Text, int32& OutValue);

} // namespace Kingdom
