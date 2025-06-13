#ifndef ELAPSEDTIMER_H
#define ELAPSEDTIMER_H

#include <chrono>

class ElapsedTimer {
private:
	std::chrono::high_resolution_clock::time_point start_;
public:
	void start()
	{
		start_ = std::chrono::high_resolution_clock::now();
	}
	unsigned long long elapsed()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
		return duration.count();
	}
};

#endif // ELAPSEDTIMER_H
