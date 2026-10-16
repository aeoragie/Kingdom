#pragma once

#include "KingdomCore/EnumString.h"

namespace Kingdom
{

// 지원하는 문자 인코딩. 인코딩을 더할 때 이 목록 한 줄과 Encoding.cpp의 구현 클래스 하나를 더한다.
// 코어의 문자열과 데이터 파일은 UTF-8만 쓴다. 다른 인코딩은 쓸 곳이 생기면(예: 옛 파일 가져오기) 더한다.
#define KINGDOM_ENCODINGS(X) \
	X(Utf8)

KINGDOM_DEFINE_ENUM(EEncoding, EncodingCount, KINGDOM_ENCODINGS)

// 인코딩 하나의 규칙. 상태가 없다.
// 인터페이스에는 지금 쓰는 것만 둔다. 문자 단위 풀기·변환(Decode/Encode)은 필요해질 때 더한다.
class IEncoding
{
public:
	virtual ~IEncoding() = default;

	virtual EEncoding GetType() const = 0;

	// 사람이 읽는 이름(오류 메시지용). 예) "UTF-8"
	virtual const char* GetName() const = 0;

	// 바이트열이 이 인코딩으로 올바른지
	virtual bool IsValid(const string& Bytes) const = 0;

	// 앞에 붙은 BOM의 바이트 수. 없거나 BOM이 없는 인코딩이면 0
	virtual size_t GetBomLength(const string& Bytes) const = 0;
};

// 인코딩 객체를 얻는다. 인코딩마다 하나뿐인 정적 객체라 만들거나 지울 필요가 없다.
// 구현 클래스는 공개하지 않는다(바깥은 IEncoding만 안다).
KINGDOMCORE_API const IEncoding& GetEncoding(EEncoding Type);

} // namespace Kingdom
