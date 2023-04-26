#pragma once
#include <Windows.h>
#include <stdint.h>


class Timer
{
public:
    Timer();
    ~Timer();

    void Tick();
    double DeltaTime();

private:
    double seconds_per_count_ = 0.0;
    int64_t cur_time_ = 0;
    int64_t pre_time_ = 0;
    double delta_time_ = 0.0;
};


