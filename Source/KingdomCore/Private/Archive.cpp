#include "KingdomCore/Archive.h"

namespace Kingdom
{

namespace
{

// 문자열·바이트 묶음 하나의 최대 길이. 깨진 세이브가 거대한 길이를 읽어 메모리를 터뜨리지 않게
constexpr uint32 MaxBlobLength = 256u * 1024u * 1024u;

} // namespace

void FArchiveWriter::WriteUInt8(uint8 Value)
{
	Buffer.push_back(Value);
}

void FArchiveWriter::WriteUInt32(uint32 Value)
{
	for (int32 Shift = 0; Shift < 32; Shift += 8)
	{
		Buffer.push_back(static_cast<uint8>(Value >> Shift));
	}
}

void FArchiveWriter::WriteUInt64(uint64 Value)
{
	for (int32 Shift = 0; Shift < 64; Shift += 8)
	{
		Buffer.push_back(static_cast<uint8>(Value >> Shift));
	}
}

void FArchiveWriter::WriteInt32(int32 Value)
{
	WriteUInt32(static_cast<uint32>(Value));
}

void FArchiveWriter::WriteInt64(int64 Value)
{
	WriteUInt64(static_cast<uint64>(Value));
}

void FArchiveWriter::WriteBool(bool bValue)
{
	WriteUInt8(bValue ? 1 : 0);
}

void FArchiveWriter::WriteString(const string& Value)
{
	WriteUInt32(static_cast<uint32>(Value.size()));
	Buffer.insert(Buffer.end(), Value.begin(), Value.end());
}

void FArchiveWriter::WriteBytes(const vector<uint8>& Bytes)
{
	WriteUInt32(static_cast<uint32>(Bytes.size()));
	Buffer.insert(Buffer.end(), Bytes.begin(), Bytes.end());
}

bool FArchiveReader::CanRead(size_t Count)
{
	if (bError || Buffer.size() - Position < Count)
	{
		bError = true;
		return false;
	}
	return true;
}

uint8 FArchiveReader::ReadUInt8()
{
	if (!CanRead(1))
	{
		return 0;
	}
	return Buffer[Position++];
}

uint32 FArchiveReader::ReadUInt32()
{
	if (!CanRead(4))
	{
		return 0;
	}
	uint32 Value = 0;
	for (int32 Shift = 0; Shift < 32; Shift += 8)
	{
		Value |= static_cast<uint32>(Buffer[Position++]) << Shift;
	}
	return Value;
}

uint64 FArchiveReader::ReadUInt64()
{
	if (!CanRead(8))
	{
		return 0;
	}
	uint64 Value = 0;
	for (int32 Shift = 0; Shift < 64; Shift += 8)
	{
		Value |= static_cast<uint64>(Buffer[Position++]) << Shift;
	}
	return Value;
}

int32 FArchiveReader::ReadInt32()
{
	return static_cast<int32>(ReadUInt32());
}

int64 FArchiveReader::ReadInt64()
{
	return static_cast<int64>(ReadUInt64());
}

bool FArchiveReader::ReadBool()
{
	const uint8 Value = ReadUInt8();
	if (Value > 1)
	{
		bError = true;
		return false;
	}
	return Value == 1;
}

string FArchiveReader::ReadString()
{
	const uint32 Length = ReadUInt32();
	if (Length > MaxBlobLength || !CanRead(Length))
	{
		bError = true;
		return string();
	}
	string Value(reinterpret_cast<const char*>(Buffer.data() + Position), Length);
	Position += Length;
	return Value;
}

vector<uint8> FArchiveReader::ReadBytes()
{
	const uint32 Length = ReadUInt32();
	if (Length > MaxBlobLength || !CanRead(Length))
	{
		bError = true;
		return vector<uint8>();
	}
	vector<uint8> Value(Buffer.begin() + static_cast<ptrdiff_t>(Position), Buffer.begin() + static_cast<ptrdiff_t>(Position + Length));
	Position += Length;
	return Value;
}

} // namespace Kingdom
