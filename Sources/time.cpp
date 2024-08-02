#include "../Headers/time.hpp"

size_t Time::get_correct_time(const size_t& myTimeRemainingMs, const size_t& myIncrementMs)
{
	// Get a fraction of remaining time to use for current move
	double thinkTimeMs = myTimeRemainingMs / (40.0);

	// Clamp think time if a maximum limit is imposed
	if (useMaxThinkTime) { thinkTimeMs = std::min(static_cast<double>(maxThinkTimeMs), thinkTimeMs); }

	// Add increment
	if (myTimeRemainingMs > myIncrementMs * 2) { thinkTimeMs += myIncrementMs * 0.8; }

	double minThinkTime = std::min(static_cast<double>(50), myTimeRemainingMs * 0.25);
	return static_cast<int>(std::ceil(std::max(minThinkTime, thinkTimeMs)));
}
