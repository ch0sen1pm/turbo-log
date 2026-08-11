#pragma once

#include "crypt/crypt.h"

namespace logger {
namespace crypt {
class AESCrypt final : public Crypt {
public:
    explicit AESCrypt(std::string key);
    ~AESCrypt() override = default;

    static std::string GenerateKey();
    static std::string GenerateIV();

    void Encrypt(const void* input, size_t input_size,
                 std::string& output) override;

    std::string Decrypt(const void* data, size_t size) override;

private:
    std::string key_;
    std::string iv_;
};

} // namespace crypt
} // namespace logger