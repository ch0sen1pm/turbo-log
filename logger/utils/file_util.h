#pragma once

#include <stdint.h>
#include <filesystem>

namespace logger {
namespace fs {

size_t GetFileSize(const std::filesystem::path& file_path);

} // namespace fs

} // namespace logger 