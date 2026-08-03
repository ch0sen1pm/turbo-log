#include "log_factory.h"

namespace logger {
LogFactory::LogFactory() {}

LogFactory& LogFactory::Instance() {
    static LogFactory instance;
    return instance;
}

VariadicLogHandle* LogFactory::GetLogHandle() {
    return log_handle_.get();
}

void LogFactory::SetLogHandle(std::shared_ptr<VariadicLogHandle> log_handle) {
    log_handle_ = log_handle;
}

}