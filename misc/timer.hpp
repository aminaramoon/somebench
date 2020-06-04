#pragma once

#include <chrono>
#include <thread>
#include <iostream>

struct Timer
{
    using clock_t = std::chrono::steady_clock;

    Timer(std::chrono::milliseconds dur, std::chrono::milliseconds timeout) : duration_{dur}, timeout_{timeout}
    {
    }

    void Begin()
    {
        sleep_till_ = clock_t::now();
        end_time_ = sleep_till_ + timeout_;
        is_started_ = true;
    }

    bool IsReady()
    {
        std::this_thread::sleep_until(sleep_till_);
        sleep_till_ += duration_;
        return end_time_ > sleep_till_;
    }

private:
    bool is_started_{false};
    clock_t::time_point sleep_till_, end_time_;
    std::chrono::milliseconds timeout_, duration_;
};