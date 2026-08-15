#include "compress/zstd_compress.h"

#include <cstring>

namespace logger {
namespace compress {

ZstdCompress::ZstdCompress() {
    cctx_ = ZSTD_createCCtx();
    ZSTD_CCtx_setParameter(cctx_, ZSTD_c_compressionLevel, 5);
    dctx_ = ZSTD_createDCtx();
}

ZstdCompress::~ZstdCompress() {
    if (cctx_) {
        ZSTD_freeCCtx(cctx_);
    }
    if (dctx_) {
        ZSTD_freeDCtx(dctx_);
    }
}

void ZstdCompress::ResetStream() {
    if (cctx_) {
        ZSTD_CCtx_reset(cctx_, ZSTD_reset_session_only);
    }
}

void ZstdCompress::ResetUncompressStream_() {
    if (dctx_) {
        ZSTD_DCtx_reset(dctx_, ZSTD_reset_session_only);
    }
}

static bool IsZSTDCompressed(const void* input, size_t input_size) {
    if (!input) {
        return false;
    }
    if (input_size < 4) {
        return false;
    }

    const uint8_t kMagicNumberBigEndian[] = {0x28, 0xb5, 0x2f, 0xfd};
    const uint8_t kMagicNumberLittleEndian[] = {0xfd, 0x2f, 0xb5, 0x28};

    if (memcmp(input, kMagicNumberBigEndian, 4) == 0) {
        return true;
    }
    if (memcmp(input, kMagicNumberLittleEndian, 4) == 0) {
        return true;
    } 
    return false;
}

size_t ZstdCompress::Compress(const void* data, size_t size,
                              void* output, size_t output_size) {
    if (!data || size == 0) {
        return 0;
    }

    ZSTD_inBuffer input = {data, size, 0};
    ZSTD_outBuffer output_buffer = {
        const_cast<void*>(reinterpret_cast<const void*>(output)), output_size, 0
    };

    size_t ret = ZSTD_compressStream2(cctx_, &output_buffer, &input, ZSTD_e_flush);
    if (ZSTD_isError(ret) != 0) {
        return 0;
    }

    return output_buffer.pos;
}

std::string ZstdCompress::Decompress(const void* data, size_t size) {
    if (!data || size == 0) {
        return "";
    }

    if (IsZSTDCompressed(data, size)) {
        ResetUncompressStream_();
    }

    std::string output;
    output.reserve(10 * 1024);

    ZSTD_inBuffer input  = {data, size, 0};
    ZSTD_outBuffer output_buffer = {
        const_cast<void*>(reinterpret_cast<const void*>(output.data())), output.capacity(), 0
    };

    size_t ret = ZSTD_decompressStream(dctx_, &output_buffer, &input);
    if (ZSTD_isError(ret) != 0) {
        return "";
    }
    output = std::string(reinterpret_cast<char*>(output_buffer.dst), output_buffer.pos);
    return output;
}

size_t ZstdCompress::CompressedBound(size_t input_size) {
    return ZSTD_compressBound(input_size);
}


} // namespace logger
} // namespace compress