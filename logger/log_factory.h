#pragma once

#include <memory>

namespace logger {
class VariadicLogHandle;
class LogFactory {
public:
    LogFactory(const LogFactory&) = delete;
    LogFactory& operator=(const LogFactory&) = delete;

    static LogFactory& Instance();

    VariadicLogHandle* GetLogHandle();

    void SetLogHandle(std::shared_ptr<VariadicLogHandle> log_handle);

private:
    LogFactory();

    std::shared_ptr<VariadicLogHandle> log_handle_;
};

} // namespace logger