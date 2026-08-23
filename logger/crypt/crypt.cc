#include "crypt/crypt.h"

#include <stdexcept>

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
namespace crypt {

using CryptoPP::byte;

std::string BinaryKeyToHex(const std::string& binary_key) {
    std::string hex_key;
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(hex_key));
    encoder.Put(reinterpret_cast<const byte*>(binary_key.data()), binary_key.size());
    encoder.MessageEnd();
    return hex_key;
}

std::string HexKeyToBinary(const std::string& hex_key) {
    std::string binary_key;
    CryptoPP::HexDecoder decoder;
    decoder.Attach(new CryptoPP::StringSink(binary_key));
    decoder.Put(reinterpret_cast<const byte*>(hex_key.data()), hex_key.size());
    decoder.MessageEnd();
    return binary_key;
}

std::tuple<std::string, std::string> GenECDHKey() {
    CryptoPP::AutoSeededRandomPool rnd;
    CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());

    CryptoPP::SecByteBlock priv(dh.PrivateKeyLength());
    CryptoPP::SecByteBlock pub(dh.PublicKeyLength());

    dh.GenerateKeyPair(rnd, priv, pub);

    return std::make_tuple(
        std::string(reinterpret_cast<const char*>(priv.data()), priv.size()),
        std::string(reinterpret_cast<const char*>(pub.data()), pub.size()));
}

std::string GenECDHSharedSecret(const std::string& client_pri,
                               const std::string& server_pub) {
    CryptoPP::AutoSeededRandomPool rnd;
    CryptoPP::ECDH<CryptoPP::ECP>::Domain dh(CryptoPP::ASN1::secp256r1());

    CryptoPP::SecByteBlock shared(dh.AgreedValueLength());
    CryptoPP::SecByteBlock pub(reinterpret_cast<const byte*>(server_pub.data()), server_pub.size());
    CryptoPP::SecByteBlock priv(reinterpret_cast<const byte*>(client_pri.data()), client_pri.size());

    if (!dh.Agree(shared, priv, pub)) {
        throw std::runtime_error("Failed to reach shared secret");
    }

    return std::string(reinterpret_cast<const char*>(shared.data()), shared.size());
}

} // namespace crypt
} // namespace logger