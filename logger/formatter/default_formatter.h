#pragma once

#include "formatter/formatter.h"

namespace logger {

class DefaultFormatter : public Formatter {
public:
    void Format(const LogMsg& msg, MemoryBuf* dest) override;
};

} // namespace logger