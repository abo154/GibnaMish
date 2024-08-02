#pragma once
#ifndef _TIME_HPP_
#define _TIME_HPP_

#include <cmath>

class Time
{
public:
	size_t get_correct_time(const size_t&, const size_t&);
private:
	bool useMaxThinkTime = false;
	int maxThinkTimeMs = 2500;
};

#endif // !_TIME_HPP_