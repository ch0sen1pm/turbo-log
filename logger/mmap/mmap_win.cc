#include "mmap/mmap_aux.h"

#include <windows.h>

namespace logger {
class FileHandleDeleter {
public:
    void operator()(HANDLE h) {
        if (h != NULL) {
            CloseHandle(h);
        }
    }
};

using FileHandlePtr = std::unique_ptr<std::remove_pointer_t<HANDLE>, FileHandleDeleter>;

bool MMapAux::TryMap_(size_t capacity) {
    FileHandlePtr file_handle(
        CreateFileW(file_path_.wstring().c_str(),  // 宽字符路径
                    GENERIC_READ | GENERIC_WRITE,   // 可读可写
                    0,                              // 不共享
                    NULL,                           // 默认安全属性
                    OPEN_ALWAYS,                    // 打开或创建
                    FILE_ATTRIBUTE_NORMAL,          // 普通文件
                    NULL));
    if (file_handle.get() == INVALID_HANDLE_VALUE) {
        return false;
    }

    FileHandlePtr file_mapping_handle;
    file_mapping_handle.reset(
        CreateFileMapping(file_handle.get(),   // 文件句柄
                        NULL,                // 默认安全属性
                        PAGE_READWRITE,      // 可读可写页
                        0,                   // 大小高 32 位
                        capacity,            // 大小低 32 位（capacity 在 32 位范围够用）
                        NULL));
    if (!file_mapping_handle.get()) {
        return false;
    }

    handle_ = MapViewOfFile(file_mapping_handle.get(),  // 映射对象
                            FILE_MAP_ALL_ACCESS,         // 完全访问
                            0, 0,                        // 偏移量 = 0
                            capacity);                   // 映射大小

    return handle_ != NULL;
}

void MMapAux::Unmap_() {
    if (handle_) {
        UnmapViewOfFile(handle_);
    }
    handle_ = NULL;
}

void MMapAux::Sync_() {
    if (handle_) {
        FlushViewOfFile(handle_, capacity_);
    }
}

} // namespace logger