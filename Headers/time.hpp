#pragma once
#ifndef _TIME_HPP_
#define _TIME_HPP_

#include <cstdint>

class Timer
{
public:
	static size_t get_correct_time(size_t, size_t, size_t);
private:
	static bool useMaxThinkTime;
	static int maxThinkTimeMs;
};

#endif // !_TIME_HPP_