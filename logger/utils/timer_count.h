#pragma once

#include <chrono>
#include <string>

#include "internal_log.h"

namespace logger {

class TimerCount {
public:
    TimerCount(std::string_view info) : info_(info), start_(std::chrono::steady_clock::now()) {}

    ~TimerCount() {
        auto end = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    }
private:
    std::string_view info_;
    std::chrono::time_point<std::chrono::steady_clock> start_;
};

} // namespace logger

#define TIMER_COUNT(info) void(0)