#include "sinks/effective_sink.h"

#include <fstream>

#include "compress/zstd_compress.h"
#include "crypt/aes_crypt.h"
#include "defer.h"
#include "formatter/effective_formatter.h"
#include "internal_log.h"
#include "utils/file_util.h"
#include "utils/sys_util.h"
#include "utils/timer_count.h"

namespace logger {

EffectiveSink::EffectiveSink(Conf conf) : conf_(std::move(conf)) {
    if (!std::filesystem::exists(conf_.dir)) {
        std::filesystem::create_directories(conf_.dir);
    }

    task_runner_ = NEW_TASK_RUNNER(10086);

    formatter_ = std::make_unique<EffectiveFormatter>();

    auto ecdh_key = crypt::GenECDHKey();
    auto client_pri = std::get<0>(ecdh_key);
    client_pub_key_ = std::get<1>(ecdh_key);

    std::string svr_pub_key_bin = crypt::HexKeyToBinary(conf_.pub_key);
    std::string shared_secret = crypt::GenECDHSharedSecret(client_pri, svr_pub_key_bin);

    crypt_ = std::make_unique<crypt::AESCrypt>(shared_secret);

    compress_ = std::make_unique<compress::ZstdCompress>();

    master_cache_ = std::make_unique<MMapAux>(conf_.dir / "master_cache");
    slave_cache_ = std::make_unique<MMapAux>(conf_.dir / "slave_cache");

    if (!master_cache_ || !slave_cache_) {
        throw std::runtime_error("EffectiveSink::EffectiveSink: create mmap failed");
    }

    if (!slave_cache_->Empty()) {
        is_slave_free_.store(false);
        PrepareToFile_();
        WAIT_TASK_IDLE(task_runner_);
    }

    if (!master_cache_->Empty()) {
        if (is_slave_free_.load()) {
            is_slave_free_.store(false);
            SwapCache_();
        }
        PrepareToFile_();
    }

    POST_REPEATED_TASK(task_runner_, [this]() { ElimateFiles_(); }, conf_.interval, -1);
}

}
