#pragma once

#include "log_common.h"

namespace logger {
struct LogMsg {
    LogMsg(SourceLocation loc, LogLevel lvl, StringView msg);

    LogMsg(LogLevel lvl, StringView msg);

    LogMsg(const LogMsg& other) = default;
    LogMsg& operator=(const LogMsg& other) = default;

    SourceLocation location;
    LogLevel       level;
    StringView     message;
};

} // namespace logger