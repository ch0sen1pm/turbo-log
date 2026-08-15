if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(UTIL_SRCS utils/sys_util_linux.cc)
    set(MMAP_SRCS mmap/mmap_aux.cc mmap/mmap_linux.cc)
else()
    set(UTIL_SRCS utils/sys_util_win.cc)
    set(MMAP_SRCS mmap/mmap_aux.cc mmap/mmap_win.cc)
endif()

set(SRCS
    log_msg.cc
    log_handle.cc
    log_factory.cc
    formatter/default_formatter.cc
    sinks/console_sink.cc
    crypt/crypt.cc
    crypt/aes_crypt.cc
    compress/zstd_compress.cc
    ${UTIL_SRCS}
    ${MMAP_SRCS}
)

add_library(turbo-log STATIC ${SRCS})
target_include_directories(turbo-log PUBLIC .)
target_link_libraries(turbo-log PUBLIC fmt::fmt cryptopp::cryptopp zstd::libzstd)
