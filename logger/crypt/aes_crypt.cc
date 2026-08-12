#include "crypt/aes_crypt.h"

#include "cryptopp/aes.h"
#include "cryptopp/base64.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/oids.h"
#include "cryptopp/osrng.h"

namespace logger {
namespace crypt{
namespace detail{

using CryptoPP::byte;
static std::string GenerateKey() {
    CryptoPP::AutoSeededRandomPool rnd;
    byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
    rnd.GenerateBlock(key, sizeof(key));
    return BinaryKeyToHex(std::string(reinterpret_cast<const char*>(key), sizeof(key)));
}

static std::string GenerateIV() {
    CryptoPP::AutoSeededRandomPool rnd;
    byte iv[CryptoPP::AES::BLOCKSIZE];
    rnd.GenerateBlock(iv, sizeof(iv));
    return BinaryKeyToHex(std::string(reinterpret_cast<const char*>(iv), sizeof(iv)));
}

void Encrypt(const void* input, size_t input_size, std::string& output,
             const std::string& key, const std::string&iv) {
    
    CryptoPP::AES::Encryption aes_encryption(
        reinterpret_cast<const byte*>(key.data()), key.size()
    );

    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbc_encryption(
        aes_encryption, reinterpret_cast<const byte*>(iv.data())
    );

    CryptoPP::StreamTransformationFilter stf_encryptor(
        cbc_encryption, new CryptoPP::StringSink(output)
    );

    stf_encryptor.Put(reinterpret_cast<const byte*>(input), input_size);
    stf_encryptor.MessageEnd();
}

static std::string Decrypt(const void* data, size_t size,
                           const std::string& key, const std::string& iv) {
    std::string decryptedtext;

    CryptoPP::AES::Decryption aes_decryption(
        reinterpret_cast<const byte*>(key.data()), key.size()
    );

    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbc_decryption(
        aes_decryption, reinterpret_cast<const byte*>(iv.data())
    );

    CryptoPP::StreamTransformationFilter stf_decryptor(
        cbc_decryption, new CryptoPP::StringSink(decryptedtext)
    );
    stf_decryptor.Put(reinterpret_cast<const byte*>(data), size);
    stf_decryptor.MessageEnd();

    return decryptedtext;
}
} // namesapce detail
AESCrypt::AESCrypt(std::string key) {
    key_ = std::move(key);
    iv_ = "dad0c0012340080a";
}

void AESCrypt::Encrypt(const void* input, size_t input_size, std::string& output) {
    detail::Encrypt(input, input_size, output, key_, iv_);
}

std::string AESCrypt::Decrypt(const void* data, size_t size) {
    return detail::Decrypt(data, size, key_, iv_);
}

std::string AESCrypt::GenerateKey() {
    return detail::GenerateKey();
}

std::string AESCrypt::GenerateIV() {
    return detail::GenerateIV();
}
} // namespace crypt
} // namespace logger 