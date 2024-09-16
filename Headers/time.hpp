#pragma once
#ifndef _TIME_HPP_
#define _TIME_HPP_

#include "types.hpp"

class Timer
{
public:
	size_t get_correct_time(size_t, size_t, size_t);
private:
	bool useMaxThinkTime = false;
	int maxThinkTimeMs = 2500;
};

#endif // !_TIME_HPP_