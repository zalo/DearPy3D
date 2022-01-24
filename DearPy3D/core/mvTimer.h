#pragma once

#include <chrono>

struct mvTimer
{
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point last;

	mvTimer();

	float mark();
	float peek() const;
	float now() const;
	
};