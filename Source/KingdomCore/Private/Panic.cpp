#include "KingdomCore/Panic.h"

namespace Kingdom
{

void Panic(const string& Message)
{
	std::fprintf(stderr, "[Kingdom] PANIC: %s\n", Message.c_str());
	std::fflush(stderr);
	std::abort();
}

} // namespace Kingdom
