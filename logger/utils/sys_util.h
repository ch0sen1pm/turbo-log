#pragma once

#include <time.h>
#include <cstdint>
#include <ctime>
#include <string>

namespace logger {

size_t GetPageSize();

size_t GetProcessId();

size_t GetThreadId();

void LocalTime(std::tm* tm, std::time_t* now);


} // namespace logger