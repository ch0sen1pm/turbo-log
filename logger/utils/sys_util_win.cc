#include "utils/sys_util.h"

#include <windows.h>
#include <sysinfoapi.h>

namespace logger {
size_t GetPageSize() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

size_t GetProcessId() {
    return static_cast<size_t>(::GetCurrentProcessId());
}

size_t GetThreadId() {
    return static_cast<size_t>(::GetCurrentThreadId());
}

void LocalTime(std::tm* tm, std::time_t* now) {
    localtime_s(tm, now);
}

}