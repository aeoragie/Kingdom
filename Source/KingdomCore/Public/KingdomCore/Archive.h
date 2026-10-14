#pragma once

#include "KingdomCore/CoreTypes.h"

namespace Kingdom
{

// 세이브용 바이트 스트림. 정수는 항상 리틀 엔디언으로 쓴다(플랫폼이 달라도 같은 바이트).
// 부동소수점은 일부러 지원하지 않는다. 코어 상태는 정수만 쓴다(결정론).
//
// 언리얼의 FArchive(FMemoryWriter / FMemoryReader)와 같은 개념이지만,
// 코어는 UE 헤더를 쓸 수 없어서 따로 둔다.
class FArchiveWriter
{
public:
	KINGDOMCORE_API void WriteUInt8(uint8 Value);
	KINGDOMCORE_API void WriteUInt32(uint32 Value);
	KINGDOMCORE_API void WriteUInt64(uint64 Value);
	KINGDOMCORE_API void WriteInt32(int32 Value);
	KINGDOMCORE_API void WriteInt64(int64 Value);
	KINGDOMCORE_API void WriteBool(bool bValue);
	KINGDOMCORE_API void WriteString(const string& Value);
	KINGDOMCORE_API void WriteBytes(const vector<uint8>& Bytes);

	const vector<uint8>& GetBuffer() const { return Buffer; }
	vector<uint8> TakeBuffer() { return std::move(Buffer); }

private:
	vector<uint8> Buffer;
};

// 읽다가 데이터가 모자라거나 형식이 틀리면 오류 상태가 되고, 이후 읽기는 0을 돌려준다.
// 호출하는 쪽은 다 읽은 뒤 HasError()만 확인하면 된다.
class FArchiveReader
{
public:
	explicit FArchiveReader(const vector<uint8>& InBuffer)
		: Buffer(InBuffer)
	{
	}

	KINGDOMCORE_API uint8 ReadUInt8();
	KINGDOMCORE_API uint32 ReadUInt32();
	KINGDOMCORE_API uint64 ReadUInt64();
	KINGDOMCORE_API int32 ReadInt32();
	KINGDOMCORE_API int64 ReadInt64();
	KINGDOMCORE_API bool ReadBool();
	KINGDOMCORE_API string ReadString();
	KINGDOMCORE_API vector<uint8> ReadBytes();

	void SetError() { bError = true; }
	bool HasError() const { return bError; }
	bool IsAtEnd() const { return Position == Buffer.size(); }

private:
	bool CanRead(std::size_t Count);

	const vector<uint8>& Buffer;
	std::size_t Position = 0;
	bool bError = false;
};

} // namespace Kingdom
