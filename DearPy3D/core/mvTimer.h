#pragma once
#include <chrono>

class mvTimer
{

public:

	mvTimer();

	float mark();
	float peek() const;
	float now() const;
	
private:

	std::chrono::steady_clock::time_point m_start;
	std::chrono::steady_clock::time_point m_last;
};