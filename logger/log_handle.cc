#include "log_handle.h"
#include "sinks/sink.h"

namespace logger {

LogHandle::LogHandle(LogSinkPtrList sinks) : level_{LogLevel::kInfo} {
    for (auto& sink : sinks) {
        sinks_.push_back(std::move(sink));
    }
}

LogHandle::LogHandle(LogSinkPtr sink) : level_{LogLevel::kInfo} {
    sinks_.push_back(std::move(sink));
}

void LogHandle::SetLevel(LogLevel level) {
    level_ = level;
}

LogLevel LogHandle::GetLevel() const {
    return level_;
}

void LogHandle::Log(LogLevel level, SourceLocation loc, StringView message) {
    if (!ShouldLog_(level)) {
        return ;
    }

    LogMsg msg(loc, level, message);
    Log_(msg);
}

void LogHandle::Log_(const LogMsg& log_msg) {
    for (auto& sink : sinks_) {
        sink->Log(log_msg);
    }
}


} // namespace logger