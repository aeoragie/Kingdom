#include "MiniTest.h"

#include "KingdomCore/Encoding.h"

#include <string>

namespace
{

const Kingdom::IEncoding& Utf8()
{
	return Kingdom::GetEncoding(Kingdom::EEncoding::Utf8);
}

} // namespace

TEST_CASE(Encoding_Utf8IdentifiesItself)
{
	CHECK(Utf8().GetType() == Kingdom::EEncoding::Utf8);
	CHECK(std::string(Utf8().GetName()) == "UTF-8");

	Kingdom::EEncoding Parsed = Kingdom::EEncoding::Utf8;
	CHECK(Kingdom::StringToEnum(Kingdom::EnumToString(Kingdom::EEncoding::Utf8), Parsed));
	CHECK(Parsed == Kingdom::EEncoding::Utf8);
}

TEST_CASE(Encoding_Utf8AcceptsValidText)
{
	CHECK(Utf8().IsValid(""));
	CHECK(Utf8().IsValid("Plains,1"));
	CHECK(Utf8().IsValid("\xED\x8F\x89\xEC\xA7\x80"));   // "평지"
	CHECK(Utf8().IsValid("\xC2\xA9"));                    // 2바이트 ©
	CHECK(Utf8().IsValid("\xF0\x9F\x8F\xB0"));            // 4바이트 U+1F3F0
	CHECK(Utf8().IsValid("\xF4\x8F\xBF\xBF"));            // 마지막 코드 포인트 U+10FFFF
}

TEST_CASE(Encoding_Utf8RejectsCp949)
{
	CHECK(!Utf8().IsValid("\xC7\xD1"));                   // CP949 "한"
	CHECK(!Utf8().IsValid("\xC6\xF2\xC1\xF6"));           // CP949 "평지"
}

TEST_CASE(Encoding_Utf8RejectsBrokenSequences)
{
	CHECK(!Utf8().IsValid("\xED\x8F"));                   // 3바이트 중 2바이트에서 끊김
	CHECK(!Utf8().IsValid("\x80"));                       // 이어지는 바이트로 시작
	CHECK(!Utf8().IsValid("\xFF"));                       // UTF-8에 없는 바이트
	CHECK(!Utf8().IsValid("\xED\x8F" "A"));               // 이어지는 자리에 ASCII
}

TEST_CASE(Encoding_Utf8RejectsOverlongAndSurrogate)
{
	CHECK(!Utf8().IsValid("\xC0\xAF"));                   // 2바이트 과잉 표현 "/"
	CHECK(!Utf8().IsValid("\xE0\x80\xAF"));               // 3바이트 과잉 표현
	CHECK(!Utf8().IsValid("\xF0\x80\x80\xAF"));           // 4바이트 과잉 표현
	CHECK(!Utf8().IsValid("\xED\xA0\x80"));               // 서로게이트 U+D800
	CHECK(!Utf8().IsValid("\xF4\x90\x80\x80"));           // U+110000, 범위 초과
}

TEST_CASE(Encoding_Utf8BomLength)
{
	CHECK(Utf8().GetBomLength("\xEF\xBB\xBFId") == 3);
	CHECK(Utf8().GetBomLength("Id") == 0);
	CHECK(Utf8().GetBomLength("\xEF\xBB") == 0);
	CHECK(Utf8().IsValid("\xEF\xBB\xBFId"));              // BOM도 올바른 UTF-8이다
}
