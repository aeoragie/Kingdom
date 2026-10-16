#include "KingdomCore/Panic.h"

#include "KingdomCore/Check.h"

namespace Kingdom
{

void Panic(const string& Message)
{
	std::fprintf(stderr, "[Kingdom] PANIC: %s\n", Message.c_str());
	std::fflush(stderr);
	std::abort();
}

namespace Detail
{

void CheckFailed(const char* Condition, const char* File, int Line)
{
	Panic(string("check failed: ") + Condition + " (" + File + ":" + std::to_string(Line) + ")");
}

} // namespace Detail

} // namespace Kingdom
