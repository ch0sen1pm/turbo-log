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

void EffectiveSink::Log(const LogMsg& msg) {
    static thread_local MemoryBuf buf;

    formatter_->Format(msg, &buf);

    if (master_cache_->Empty()) {
        compress_->ResetStream();
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);

        compressed_buf_.reserve(compress_->CompressedBound(buf.size()));
        size_t compressed_size = 
            compress_->Compress(buf.data(), buf.size(), compressed_buf_.data(), compressed_buf_.capacity());

        if (compressed_size == 0) {
            LOG_ERROR("EffectiveSink::Log: compress failed");
            return;
        }

        encryped_buf_.clear();
        encryped_buf_.reserve(compressed_size + 16);
        crypt_->Encrypt(compressed_buf_.data(), compressed_size, encryped_buf_);
        if (encryped_buf_.empty()) {
            LOG_ERROR("EffectiveSink::Log: encrypt failed");
            return;
        }

        WriteToCache_(encryped_buf_.data(), encryped_buf_.size());
    }

    if (NeedCacheToFile_()) {
        if (is_slave_free_.load()) {
            is_slave_free_.store(false);
            SwapCache_();
        }
        PrepareToFile_();
    }
}

void EffectiveSink::SetFormatter(std::unique_ptr<Formatter> formatter) {}

void EffectiveSink::Flush() {
    TIMER_COUNT("Flush");

    PrepareToFile_();
    WAIT_TASK_IDLE(task_runner_);

    if (is_slave_free_.load()) {
        is_slave_free_.store(false);
        SwapCache_();
    }
    PrepareToFile_();
    WAIT_TASK_IDLE(task_runner_);
}

void EffectiveSink::SwapCache_() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::swap(master_cache_, slave_cache_);
}

bool EffectiveSink::NeedCacheToFile_() {
    return master_cache_->GetRatio() > 0.8;
}

void EffectiveSink::WriteToCache_(const void* data, uint32_t size) {
    detail::ItemHeader item_header;
    item_header.size = size;

    master_cache_->Push(&item_header, sizeof(item_header));
    master_cache_->Push(data, size);
}

void EffectiveSink::PrepareToFile_() {
    POST_TASK(task_runner_, [this]() { CacheToFile_(); });
}

void EffectiveSink::CacheToFile_() {
    TIMER_COUNT("CacheToFile_");

    if (is_slave_free_.load()) {
        return;
    }

    if (slave_cache_->Empty()) {
        is_slave_free_.store(true);
        return;
    }

    {
        auto file_path = GetFilePath_();

        detail::ChunkHeader chunk_header;
        chunk_header.size = slave_cache_->Size();
        memcpy(chunk_header.pub_key, client_pub_key_.data(), client_pub_key_.size());

        std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
        ofs.write(reinterpret_cast<char*>(&chunk_header), sizeof(chunk_header));
        ofs.write(reinterpret_cast<char*>(slave_cache_->Data()), chunk_header.size);
        ofs.close();
    }

    slave_cache_->Clear();
    is_slave_free_.store(true);
}

}
