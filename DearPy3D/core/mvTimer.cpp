#include "mvTimer.h"

using namespace std::chrono;

mvTimer::mvTimer()
{
	last = steady_clock::now();
	start = steady_clock::now();
}

float 
mvTimer::mark()
{
	const auto old = last;
	last = steady_clock::now();
	const duration<float> frameTime = last - old;
	return frameTime.count();
}

float 
mvTimer::peek() const
{
	return duration<float>(steady_clock::now() - last).count();
}

float 
mvTimer::now() const
{
	const duration<float> totalTime = steady_clock::now() - start;
	return totalTime.count();
}
