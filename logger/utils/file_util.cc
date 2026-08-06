#include "utils/file_util.h"


namespace logger {
namespace fs {

size_t GetFileSize(const std::filesystem::path& file_path) {
    if (std::filesystem::exists(file_path)) {
        return std::filesystem::file_size(file_path);
    }

    return 0;
}

}

} // namespace logger 