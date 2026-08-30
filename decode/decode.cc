#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#include <vector>

#include "decode_formatter.h"
#include "logger/compress/zstd_compress.h"
#include "logger/crypt/aes_crypt.h"
#include "logger/sinks/effective_sink.h"

using namespace logger;
using namespace logger::detail;

std::unique_ptr<DecodeFormatter> decode_formatter;

std::vector<char> ReadFile(const std::string& file_path) {
    std::ifstream ifs(file_path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("ReadFile: open file failed");
    }
    ifs.seekg(0, std::ios::end);
    auto size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    ifs.read(buffer.data(), size);
    return buffer;
}

void AppendDataToFile(const std::string& file_path, const std::string& data) {
    std::ofstream ofs(file_path, std::ios::binary | std::ios::app);
    ofs.write(data.data(), data.size());
}

std::unique_ptr<logger::compress::Compression> decompress;

void DecodeItemData(char* data, size_t size, crypt::Crypt* crypt, std::string& output) {
    std::string decrypted = crypt->Decrypt(data, size);
    std::string decompressed = decompress->Decompress(decrypted.data(), decrypted.size());
    EffectiveMsg msg;
    msg.ParseFromString(decompressed);
    std::string assemble;
    decode_formatter->Format(msg, assemble);
    output.append(assemble);
}

void DecodeChunkData(char* data, size_t size,
                     const std::string& cli_pub_key,
                     const std::string& svr_pri_key,
                     std::string& output) {
    std::cout << "decode chunk" << size << std::endl;

    std::string svr_pri_key_bin = crypt::HexKeyToBinary(svr_pri_key);
    std::string shared_secret = crypt::GenECDHSharedSecret(svr_pri_key_bin, cli_pub_key);
    std::unique_ptr<crypt::Crypt> crypt = std::make_unique<crypt::AESCrypt>(shared_secret);

    size_t offset = 0;
    size_t count = 0;

    while (offset < size) {
        count ++;
        if (count % 1000 == 0) {
            std::cout << "decode item" << count << std::endl;
        }

        ItemHeader* item_header = reinterpret_cast<ItemHeader*>(data + offset);
        if (item_header->magic != ItemHeader::kMagic) {
            throw std::runtime_error("DecodeChunkData: invalid item magic");    
        }

        offset += sizeof(ItemHeader);

        DecodeItemData(data + offset, item_header->size, crypt.get(), output);

        offset += item_header->size;

        output.push_back('\n');
    }
}

void DecodeFile(const std::string& input_file_path, const std::string& pri_key, const std::string& output_file_path) {
    auto input = ReadFile(input_file_path);
    if (input.size() < sizeof(ChunkHeader)) {
        throw std::runtime_error("DecodeFile: input file is too small");
    }

    auto chunk_header = reinterpret_cast<ChunkHeader*>(input.data());
    if (chunk_header->magic != ChunkHeader::kMagic) {
        throw std::runtime_error("DecodeFile: invalid file magic");
    }

    size_t offset = 0;
    size_t file_size = input.size();
    std::string output;
    output.reserve(1024 * 1024);

    while (offset < file_size) {
        ChunkHeader* chunk_header = reinterpret_cast<ChunkHeader*>(input.data() + offset);
        if (chunk_header->magic != ChunkHeader::kMagic) {
            throw std::runtime_error("DecodeFile: invalid chunk magic");
        }

        output.clear();
        offset += sizeof(ChunkHeader);

        DecodeChunkData(input.data() + offset, chunk_header->size,
                        std::string(chunk_header->pub_key, 65), pri_key, output);

        offset += chunk_header->size;

        AppendDataToFile(output_file_path, output);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: ./decode <file_path> <pri_key> <output_file>" << std::endl;
        return 1;
    }

    std::string input_file_path = argv[1];
    std::string pri_key = argv[2];
    std::string output_file_path = argv[3];

    try {
        decode_formatter = std::make_unique<DecodeFormatter>();
        decode_formatter->SetPattern("[%l][%D:%S][%p:%t][%F:%f:%#]%v");
        decompress = std::make_unique<logger::compress::ZstdCompress>();
        DecodeFile(input_file_path, pri_key, output_file_path);
    } catch (const std::exception& e) {
        std::cerr << "Decode failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}