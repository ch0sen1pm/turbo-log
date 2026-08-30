#include <memory>
#include <iostream>

#include "log_common.h"
#include "log_factory.h"
#include "log_handle.h"
#include "log_variadic_handle.h"
#include "logger.h"
#include "sinks/effective_sink.h"
#include "crypt/crypt.h"

int main() {
    // ===== 1. 生成服务端密钥对 =====
    auto server_key = logger::crypt::GenECDHKey();
    std::string server_pub = logger::crypt::BinaryKeyToHex(std::get<1>(server_key));
    std::string server_pri = logger::crypt::BinaryKeyToHex(std::get<0>(server_key));

    std::cout << "server public key: " << server_pub << std::endl;
    std::cout << "server private key: " << server_pri << std::endl;

    // ===== 2. 创建 EffectiveSink（加密+压缩+mmap 落盘）=====
    logger::EffectiveSink::Conf conf;
    conf.dir = "./log_data";
    conf.prefix = "turbo";
    conf.pub_key = server_pub;
    conf.interval = std::chrono::minutes(5);
    conf.single_size = logger::megabytes(4);
    conf.total_size = logger::megabytes(100);

    auto sink = std::make_shared<logger::EffectiveSink>(conf);

    // ===== 3. 创建 handle，挂上 EffectiveSink =====
    auto handle = std::make_shared<logger::VariadicLogHandle>(sink);

    // ===== 4. 注册到全局工厂 =====
    logger::LogFactory::Instance().SetLogHandle(handle);

    // ===== 5. 写日志 =====
    for (int i = 0; i < 100; ++i) {
        EXT_LOG_INFO("hello turbo-log, this is message {}", i);
        EXT_LOG_WARN("warning number {}", i);
        EXT_LOG_ERROR("error number {}", i);
    }

    // ===== 6. 手动 Flush，把缓存刷到磁盘 =====
    sink->Flush();

    std::cout << "logs written to ./log_data/" << std::endl;
    std::cout << "decode: ./decode/decode ./log_data/turbo_xxx.log " << server_pri << " ./log_data/decoded.txt" << std::endl;

    return 0;
}
