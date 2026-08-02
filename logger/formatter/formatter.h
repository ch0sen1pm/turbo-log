#pragma once

#include "log_common.h"
#include "log_msg.h"

namespace logger {
class Formatter {
public:
    virtual ~Formatter() = default;

    virtual void Format(const LogMsg& msg, MemoryBuf* dest) = 0;
};
} // namespace logger