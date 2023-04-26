#include "Timer.h"


Timer::Timer()
{
	int64_t counts_per_sec{0};
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&counts_per_sec));
	seconds_per_count_ = 1.0 / (double)counts_per_sec;
}

Timer::~Timer()
{

}

void Timer::Tick()
{
	int64_t cur_time;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&cur_time));
	cur_time_ = cur_time;

	delta_time_ = (cur_time_ - pre_time_) * seconds_per_count_;

	pre_time_ = cur_time_;

	if (delta_time_ < 0.0)
	{
		delta_time_ = 0.0;
	}
}

double Timer::DeltaTime()
{
	return delta_time_;
}

