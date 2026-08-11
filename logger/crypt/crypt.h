#pragma once

#include <memory>
#include <string>
#include <tuple>

namespace logger {
namespace crypt {
std::tuple<std::string, std::string> GenECDHKey();

std::string GenECDHSharedSecret(const std::string& client_pri,
                                const std::string& server_pub);

std::string BinaryKeyToHex(const std::string& binary_key);
std::string HexKeyToBinary(const std::string& hex_key);

class Crypt {
public:
    virtual ~Crypt() = default;

    virtual void Encrypt(const void* input, size_t input_size,
                         std::string& output) = 0;

    virtual std::string Decrypt(const void* data, size_t size) = 0;
};

} // namespace crypt

} // namespace logger