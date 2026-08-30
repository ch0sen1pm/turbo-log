add_executable(decode decode.cc decode_formatter.cc)

target_link_libraries(decode turbo-log)

target_include_directories(decode PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/logger
    ${CMAKE_BINARY_DIR}/logger/proto)