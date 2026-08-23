#include "formatter/effective_formatter.h"

#include <chrono>

#include "proto/effective_msg.pb.h"
#include "utils/sys_util.h"

namespace logger {

void EffectiveFormatter::Format(const LogMsg& msg, MemoryBuf* dest) {
    EffectiveMsg effective_msg;

    effective_msg.set_level(static_cast<int>(msg.level));
    effective_msg.set_timestamp(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    effective_msg.set_pid(GetProcessId());
    effective_msg.set_tid(GetThreadId());
    effective_msg.set_line(msg.location.line);
    effective_msg.set_file_name(msg.location.file_name.data(), msg.location.file_name.size());
    effective_msg.set_func_name(msg.location.func_name.data(), msg.location.func_name.size());
    effective_msg.set_log_info(msg.message.data(), msg.message.size());

    size_t len = effective_msg.ByteSizeLong();
    dest->resize(len);
    effective_msg.SerializeToArray(dest->data(), len);
}

}