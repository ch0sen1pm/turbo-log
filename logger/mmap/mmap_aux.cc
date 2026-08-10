#include "mmap/mmap_aux.h"

#include <string.h>

#include "utils/file_util.h"
#include "utils/sys_util.h"

namespace logger {
static constexpr size_t kDefaultCapacity = 512 * 1024;

MMapAux::MMapAux(fpath file_path)
    : file_path_(std::move(file_path))
    , handle_(nullptr)
    , capacity_(0)
{
    size_t file_size = fs::GetFileSize(file_path_);
    size_t dst_size = std::max(file_size, kDefaultCapacity);
    Reserve_(dst_size);
    Init_();
}

void MMapAux::Init_() {
    MmapHeader* header = Header_();
    if (!header) {
        return;
    }
    if (header->magic != MmapHeader::kMagic) {
        header->magic = MmapHeader::kMagic;
        header->size = 0;
    }
}

void MMapAux::Resize(size_t new_size) {
    if (!IsValid_()) {
        return;
    }
    EnsureCapacity_(new_size);
    Header_()->size = new_size;
}

void MMapAux::Clear() {
    if (!IsValid_()) {
        return;
    }
    Header_()->size = 0;
}

uint8_t* MMapAux::Data() const {
    if (!IsValid_()) {
        return nullptr;
    }
    return static_cast<uint8_t*>(handle_) + sizeof(MmapHeader);
}

size_t MMapAux::Size() const {
    if (!IsValid_()) {
        return 0;
    }
    return Header_()->size;
}

double MMapAux::GetRatio() const {
    if (!IsValid_()) {
        return 0.0;
    }
    return static_cast<double>(Size()) / (Capacity_() - sizeof(MmapHeader));
}

void MMapAux::Push(const void* data, size_t size) {
    if (!IsValid_()) {
        return;
    }
    size_t new_size = Size() + size;
    EnsureCapacity_(new_size);
    memcpy(Data() + Size(), data, size);
    Header_()->size = new_size;
}

MMapAux::MmapHeader* MMapAux::Header_() const {
    if (!handle_) {
        return nullptr;
    }
    if (capacity_ < sizeof(MmapHeader)) {
        return nullptr;
    }
    return static_cast<MmapHeader*>(handle_);
}

bool MMapAux::IsValid_() const {
    MmapHeader* header = Header_();
    if (!header) {
        return false;
    }
    return header->magic == MmapHeader::kMagic;
}

static size_t GetValidCapacity_(size_t size) {
    size_t page_size = GetPageSize();

    return (size + page_size - 1) / page_size * page_size;
}

void MMapAux::Reserve_(size_t new_size) {
    if (new_size <= capacity_) {
        return;
    }
    new_size = GetValidCapacity_(new_size);
    if (new_size == capacity_) {
        return;
    }
    Unmap_();
    TryMap_(new_size);
    capacity_ = new_size;
}

void MMapAux::EnsureCapacity_(size_t new_size) {
    size_t real_size = new_size + sizeof(MmapHeader);
    if (real_size <= capacity_) {
        return;
    }

    auto dst_capacity = capacity_;
    while (dst_capacity < real_size) {
        dst_capacity += GetPageSize();
    }
    Reserve_(dst_capacity);
}


} // namespace logger 