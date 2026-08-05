#pragma once

#include "sinks/sink.h"

namespace logger {
class ConsoleSink : public LogSink {
public:
    ConsoleSink();

    ~ConsoleSink() override = default;

    void Log(const LogMsg& msg) override;

    void SetFormatter(std::unique_ptr<Formatter> formatter) override;

private:
    std::unique_ptr<Formatter> formatter_;
};
    
}  // namespace logger 

