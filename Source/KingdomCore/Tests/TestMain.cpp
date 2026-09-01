#include "MiniTest.h"

#include "KingdomCore/SimClock.h"
#include "KingdomCore/Version.h"

#include <cstring>

TEST_CASE(CoreVersion_IsNonEmpty)
{
	CHECK(Kingdom::CoreVersion() != nullptr);
	CHECK(std::strlen(Kingdom::CoreVersion()) > 0);
}

TEST_CASE(SimClock_StartsAtZero)
{
	Kingdom::FSimClock Clock;
	CHECK(Clock.CurrentTick() == 0);
}

TEST_CASE(SimClock_AdvanceAccumulates)
{
	Kingdom::FSimClock Clock;
	Clock.Advance(3);
	Clock.Advance(0);
	Clock.Advance(7);
	CHECK(Clock.CurrentTick() == 10);
}

int main()
{
	return MiniTest::RunAll();
}
