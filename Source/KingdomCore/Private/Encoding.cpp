#include "KingdomCore/Encoding.h"

namespace Kingdom
{

namespace
{

// UTF-8 (RFC 3629). 올바른 바이트 범위만 받는다.
//   1바이트  00-7F
//   2바이트  C2-DF 80-BF                    (C0, C1은 과잉 표현이라 거절)
//   3바이트  E0    A0-BF 80-BF              (E0 80-9F는 과잉 표현)
//            E1-EC 80-BF 80-BF
//            ED    80-9F 80-BF              (ED A0-BF는 서로게이트 D800-DFFF, UTF-16 전용 값)
//            EE-EF 80-BF 80-BF
//   4바이트  F0    90-BF 80-BF 80-BF        (F0 80-8F는 과잉 표현)
//            F1-F3 80-BF 80-BF 80-BF
//            F4    80-8F 80-BF 80-BF        (F4 90 이상은 U+10FFFF 초과)
class FUtf8Encoding final : public IEncoding
{
public:
	EEncoding GetType() const override
	{
		return EEncoding::Utf8;
	}

	const char* GetName() const override
	{
		return "UTF-8";
	}

	bool IsValid(const string& Bytes) const override
	{
		size_t Index = 0;
		while (Index < Bytes.size())
		{
			const uint8 Lead = ByteAt(Bytes, Index);
			if (Lead < 0x80)
			{
				++Index;
				continue;
			}

			size_t Length = 0;
			uint8 SecondMin = 0x80;
			uint8 SecondMax = 0xBF;
			if (Lead >= 0xC2 && Lead <= 0xDF)
			{
				Length = 2;
			}
			else if (Lead >= 0xE0 && Lead <= 0xEF)
			{
				Length = 3;
				if (Lead == 0xE0)
				{
					SecondMin = 0xA0;
				}
				else if (Lead == 0xED)
				{
					SecondMax = 0x9F;
				}
			}
			else if (Lead >= 0xF0 && Lead <= 0xF4)
			{
				Length = 4;
				if (Lead == 0xF0)
				{
					SecondMin = 0x90;
				}
				else if (Lead == 0xF4)
				{
					SecondMax = 0x8F;
				}
			}
			else
			{
				return false;
			}

			if (Index + Length > Bytes.size())
			{
				return false;
			}

			const uint8 Second = ByteAt(Bytes, Index + 1);
			if (Second < SecondMin || Second > SecondMax)
			{
				return false;
			}
			for (size_t Offset = 2; Offset < Length; ++Offset)
			{
				if (!IsContinuation(ByteAt(Bytes, Index + Offset)))
				{
					return false;
				}
			}
			Index += Length;
		}
		return true;
	}

	size_t GetBomLength(const string& Bytes) const override
	{
		return Bytes.size() >= 3 && Bytes.compare(0, 3, "\xEF\xBB\xBF") == 0 ? 3 : 0;
	}

private:
	static uint8 ByteAt(const string& Bytes, size_t Index)
	{
		return static_cast<uint8>(Bytes[Index]);
	}

	static bool IsContinuation(uint8 Byte)
	{
		return (Byte & 0xC0) == 0x80;
	}
};

const FUtf8Encoding Utf8Encoding;

// EEncoding 순서와 같아야 한다
const IEncoding* const Encodings[] = {&Utf8Encoding};
static_assert(sizeof(Encodings) / sizeof(Encodings[0]) == EncodingCount, "Encodings must list every EEncoding");

} // namespace

const IEncoding& GetEncoding(EEncoding Type)
{
	const size_t Index = static_cast<size_t>(Type);
	KINGDOM_CHECK(Index < EncodingCount);
	return *Encodings[Index];
}

} // namespace Kingdom
