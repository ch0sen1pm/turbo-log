#pragma once

#include <memory>
#include <string>

namespace logger {
namespace compress {
class Compression {
public:
    virtual ~Compression() = default;

    virtual size_t Compress(const void* input, size_t input_size,
                            void* output, size_t output_size) = 0;

    virtual size_t CompressedBound(size_t input_size) = 0;

    virtual std::string Decompress(const void* data, size_t size) = 0;

    virtual void ResetStream() = 0;
};

} // namespace compress
} // namespace logger