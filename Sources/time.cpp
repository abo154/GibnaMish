#include <cmath>
#include <algorithm>

#include "../Headers/time.hpp"

bool Timer::useMaxThinkTime = false;
int Timer::maxThinkTimeMs = 2500;

size_t Timer::get_correct_time(size_t myTimeRemainingMs, size_t myIncrementMs, size_t mtg)
{
	// Get a fraction of remaining time to use for current move
	double thinkTimeMs = myTimeRemainingMs / (40.0);

	// Clamp think time if a maximum limit is imposed
	if (Timer::useMaxThinkTime) { thinkTimeMs = std::min(static_cast<double>(Timer::maxThinkTimeMs), thinkTimeMs); }

	// Add increment
	if (myTimeRemainingMs > myIncrementMs * 2) { thinkTimeMs += myIncrementMs * 0.8; }

	double minThinkTime = std::min(static_cast<double>(50), myTimeRemainingMs * 0.25);
	return static_cast<int>(std::ceil(std::max(minThinkTime, thinkTimeMs)));
}
