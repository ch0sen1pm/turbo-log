#include "log_msg.h"

namespace logger {
LogMsg::LogMsg(SourceLocation loc, LogLevel lvl, StringView msg)
    : location(std::move(loc)), level(lvl), message(std::move(msg)) {}

LogMsg::LogMsg(LogLevel lvl, StringView msg)
    : LogMsg(SourceLocation{}, lvl, msg) {}
} // namespace logger