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
    formatter/effective_formatter.cc
    sinks/console_sink.cc
    crypt/crypt.cc
    crypt/aes_crypt.cc
    compress/zstd_compress.cc
    compress/zlib_compress.cc
    ${UTIL_SRCS}
    ${MMAP_SRCS}
)

add_library(turbo-log STATIC ${SRCS})
target_include_directories(turbo-log PUBLIC .)

# 用 protoc 编译 .proto 生成 .pb.h/.pb.cc（必须在 add_library 之后）
find_package(protobuf CONFIG REQUIRED)
protobuf_generate(TARGET turbo-log PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/proto/effective_msg.proto)
# protobuf_generate 生成的文件在 CMAKE_CURRENT_BINARY_DIR，加到 include 路径
target_include_directories(turbo-log PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(turbo-log PUBLIC fmt::fmt cryptopp::cryptopp zstd::libzstd ZLIB::ZLIB protobuf::libprotobuf)
