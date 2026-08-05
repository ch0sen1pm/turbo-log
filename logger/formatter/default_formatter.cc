#include "formatter/default_formatter.h"

#include "utils/sys_util.h"

namespace logger {

void DefaultFormatter::Format(const LogMsg& msg, MemoryBuf* dest) {

    constexpr char kLogLevelStr[] = "TDIWEF";

    std::time_t now = std::time(nullptr);
    std::tm tm;
    LocalTime(&tm, &now);

    char time_buf[32] = {0};
    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);

    dest->append("[", 1);
    dest->append(time_buf, strlen(time_buf));

    dest->append("] [", 3);
    dest->append(1, kLogLevelStr[static_cast<int>(msg.level)]);

    dest->append("] [", 3);
    dest->append(msg.location.file_name.data(), msg.location.file_name.size());
    dest->append(":", 1);
    dest->append(std::to_string(msg.location.line));

    dest->append("] [", 3);
    dest->append(std::to_string(GetProcessId()));
    dest->append(":", 1);
    dest->append(std::to_string(GetThreadId()));

    dest->append("] ", 2);
    dest->append(msg.message.data(), msg.message.size());
}


} // namespace logger 