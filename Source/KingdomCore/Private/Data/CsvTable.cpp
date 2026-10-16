#include "KingdomCore/Data/CsvTable.h"

#include "KingdomCore/Encoding.h"

namespace Kingdom
{

namespace
{

bool IsEmptyRecord(const vector<string>& Fields)
{
	return Fields.size() == 1 && Fields[0].empty();
}

} // namespace

string FDataError::ToString() const
{
	string Result = Source;
	if (Line > 0)
	{
		Result += ":" + std::to_string(Line);
	}
	if (!Column.empty())
	{
		Result += " [" + Column + "]";
	}
	Result += " " + Message;
	return Result;
}

bool FCsvTable::Parse(const string& Text, const string& InSourceName, FCsvTable& OutTable, vector<FDataError>& OutErrors)
{
	OutTable = FCsvTable();
	OutTable.SourceName = InSourceName;

	const size_t ErrorCountBefore = OutErrors.size();
	auto AddError = [&](uint32 Line, const string& Message)
	{
		OutErrors.push_back({InSourceName, Line, string(), Message});
	};

	// 데이터 파일은 UTF-8만 받는다. CP949로 저장된 한글은 여기서 걸린다.
	const IEncoding& Utf8 = GetEncoding(EEncoding::Utf8);
	if (!Utf8.IsValid(Text))
	{
		AddError(0, string("not valid ") + Utf8.GetName() + " (save as 'CSV UTF-8' in Excel)");
		return false;
	}

	// BOM(엑셀의 "CSV UTF-8" 저장이 붙인다)은 건너뛴다
	size_t Position = Utf8.GetBomLength(Text);

	// 레코드(논리적인 한 행)를 하나씩 읽는다. 따옴표 안의 줄바꿈 때문에 레코드가 여러 줄일 수 있다.
	uint32 Line = 1;
	bool bHeaderRead = false;
	while (Position < Text.size())
	{
		const uint32 RecordLine = Line;
		vector<string> Fields(1);
		bool bInQuotes = false;
		bool bRecordEnded = false;

		while (Position < Text.size() && !bRecordEnded)
		{
			const char Character = Text[Position];
			if (bInQuotes)
			{
				if (Character == '"')
				{
					if (Position + 1 < Text.size() && Text[Position + 1] == '"')
					{
						Fields.back() += '"';
						Position += 2;
						continue;
					}
					bInQuotes = false;
				}
				else
				{
					if (Character == '\n')
					{
						++Line;
					}
					Fields.back() += Character;
				}
				++Position;
				continue;
			}

			switch (Character)
			{
			case '"':
				if (!Fields.back().empty())
				{
					AddError(Line, "quote in the middle of a field");
					return false;
				}
				bInQuotes = true;
				break;
			case ',':
				Fields.emplace_back();
				break;
			case '\r':
				break;
			case '\n':
				++Line;
				bRecordEnded = true;
				break;
			default:
				Fields.back() += Character;
				break;
			}
			++Position;
		}

		if (bInQuotes)
		{
			AddError(RecordLine, "unclosed quote");
			return false;
		}

		if (IsEmptyRecord(Fields))
		{
			continue;
		}

		if (!bHeaderRead)
		{
			OutTable.Header = std::move(Fields);
			bHeaderRead = true;
			for (size_t Column = 0; Column < OutTable.Header.size(); ++Column)
			{
				for (size_t Other = 0; Other < Column; ++Other)
				{
					if (OutTable.Header[Column] == OutTable.Header[Other])
					{
						AddError(RecordLine, "duplicate column '" + OutTable.Header[Column] + "'");
					}
				}
			}
			continue;
		}

		if (Fields.size() != OutTable.Header.size())
		{
			AddError(RecordLine, "has " + std::to_string(Fields.size()) + " fields, header has " + std::to_string(OutTable.Header.size()));
			continue;
		}

		OutTable.Rows.push_back(std::move(Fields));
		OutTable.RowLines.push_back(RecordLine);
	}

	if (!bHeaderRead)
	{
		AddError(0, "empty file (no header)");
	}

	return OutErrors.size() == ErrorCountBefore;
}

bool FCsvTable::FindColumn(const string& ColumnName, size_t& OutColumn) const
{
	for (size_t Column = 0; Column < Header.size(); ++Column)
	{
		if (Header[Column] == ColumnName)
		{
			OutColumn = Column;
			return true;
		}
	}
	return false;
}

bool TryParseInt32(const string& Text, int32& OutValue)
{
	if (Text.empty())
	{
		return false;
	}

	int32 Value = 0;
	const char* const Begin = Text.data();
	const char* const End = Begin + Text.size();
	const std::from_chars_result Result = std::from_chars(Begin, End, Value);
	if (Result.ec != std::errc() || Result.ptr != End)
	{
		return false;
	}

	OutValue = Value;
	return true;
}

} // namespace Kingdom
