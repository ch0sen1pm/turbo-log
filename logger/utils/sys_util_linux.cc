#include "utils/sys_util.h"

#include <unistd.h>

namespace logger {


size_t GetPageSize() {
    return getpagesize();
}

size_t GetProcessId() {
    return static_cast<size_t>(::getpid());
}

size_t GetThreadId() {
    return static_cast<size_t>(::gettid());
}

void LocalTime(std::tm* tm, std::time_t* now) {
    localtime_r(now, tm);
}

} // namespace logger