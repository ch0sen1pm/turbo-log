#pragma once

#include <memory>

#include "formatter/formatter.h"
#include "log_common.h"
#include "log_msg.h"


namespace logger {

class LogSink {
public:
    virtual ~LogSink() = default;

    virtual void Log(const LogMsg& msg) = 0;

    virtual void SetFormatter(std::unique_ptr<Formatter> formatter) = 0;

    virtual void Flush() {}
};

} // namespace logger