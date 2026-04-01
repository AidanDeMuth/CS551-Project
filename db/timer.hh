#pragma once

#include <chrono>

class Timer {
public:
	std::chrono::steady_clock::time_point start;

	Timer() : start(std::chrono::steady_clock::now()) {}

	// Returns delta in milliseconds
	double getms() {
		return std::chrono::duration<double, std::milli>(
			std::chrono::steady_clock::now() - start
		).count();
	}

	// Returns delta in seconds
	double gets() {
		return std::chrono::duration<double>(
			std::chrono::steady_clock::now() - start
		).count();
	}
};
