#pragma once

#include <chrono>

struct sTimer
{
	sTimer()
	{
		_last = std::chrono::steady_clock::now();
		_start = std::chrono::steady_clock::now();
	}

	double mark()
	{
		const auto old = _last;
		_last = std::chrono::steady_clock::now();
		const std::chrono::duration<double> frameTime = _last - old;
		return frameTime.count();
	}

	double peek()
	{
		return std::chrono::duration<double>(std::chrono::steady_clock::now() - _last).count();
	}

	double now()
	{
		const std::chrono::duration<double> totalTime = std::chrono::steady_clock::now() - _start;
		return totalTime.count();
	}

	std::chrono::steady_clock::time_point _start;
	std::chrono::steady_clock::time_point _last;
};