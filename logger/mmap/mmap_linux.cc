#include "mmap/mmap_aux.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "defer.h"

namespace logger {
bool MMapAux::TryMap_(size_t capacity) {
    int fd = open(file_path_.string().c_str(), O_RDWR | O_CREAT, S_IRWXU);

    LOG_DEFER {
        if (fd != -1) {
            close(fd);
        }
    };

    if (fd == -1) {
        return false;
    }

    ftruncate(fd, capacity);
    
    handle_ = mmap(NULL, capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    return handle_ != MAP_FAILED;
}

void MMapAux::Unmap_() {
    if (handle_) {
        munmap(handle_, capacity_);
    }
    handle_ = NULL;
}

void MMapAux::Sync_() {
    if (handle_) {
        msync(handle_, capacity_, MS_SYNC);
    }
}

}