#include <memory>

#include "log_common.h"
#include "log_factory.h"
#include "log_handle.h"
#include "log_variadic_handle.h"
#include "logger.h"
#include "sinks/console_sink.h"

int main() {
    // 1. 创建一个 ConsoleSink（输出到屏幕）
    auto sink = std::make_shared<logger::ConsoleSink>();

    // 2. 创建一个 VariadicLogHandle，把 sink 挂上去
    auto handle = std::make_shared<logger::VariadicLogHandle>(sink);

    // 3. 注册到全局工厂
    logger::LogFactory::Instance().SetLogHandle(handle);

    // 4. 第一条日志！
    EXT_LOG_INFO("hello turbo-log {}", 42);
    EXT_LOG_DEBUG("this is a debug message");
    EXT_LOG_WARN("warning: {}", "something might be wrong");
    EXT_LOG_ERROR("error code: {}", 500);

    return 0;
}