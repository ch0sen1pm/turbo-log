#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>

#include "compress/compress.h"
#include "context/context.h"
#include "crypt/crypt.h"
#include "mmap/mmap_aux.h"
#include "sinks/sink.h"
#include "space.h"

namespace logger {
namespace detail {
struct ChunkHeader {
    static constexpr uint64_t kMagic = 0xdeadbeefdada1100;
    uint64_t magic;
    uint64_t size;
    char pub_key[128];

    ChunkHeader() : magic(kMagic), size(0) {}
};

struct ItemHeader {
    static constexpr uint32_t kMagic = 0xbe5fball;
    uint32_t magic;
    uint32_t size;

    ItemHeader() : magic(kMagic), size(0) {}
};
} // namespace detail

class EffectiveSink final : public LogSink {
public:
    struct Conf {
        std::filesystem::path dir;
        std::string prefix;
        std::string pub_key;
        std::chrono::minutes interval{5};
        megabytes single_size{4};
        megabytes total_size{100}; 
    };

    explicit EffectiveSink(Conf conf);

    ~EffectiveSink() override = default;

    void Log(const LogMsg& msg) override;

    void SetFormatter(std::unique_ptr<Formatter> formatter) override;

    void Flush() override;

private:
    void SwapCache_();
    bool NeedCacheToFile_();
    void WriteToCache_(const void* data, uint32_t size);
    void PrepareToFile_();
    void CacheToFile_();
    std::filesystem::path GetFilePath_();
    void ElimateFiles_();

    Conf conf_;
    std::mutex mutex_;
    std::unique_ptr<Formatter> formatter_;
    ctx::TaskRunnerTag task_runner_;
    std::unique_ptr<crypt::Crypt> crypt_;
    std::unique_ptr<compress::Compression> compress_;
    std::unique_ptr<MMapAux> master_cache_;
    std::unique_ptr<MMapAux> slave_cache_;
    std::filesystem::path log_file_path_;
    std::string client_pub_key_;
    std::string compressed_buf_;
    std::string encryped_buf_;
    std::atomic<bool> is_slave_free_{true};
};

} // namespace logger