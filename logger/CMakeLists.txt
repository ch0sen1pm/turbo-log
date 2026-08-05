if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(UTIL_SRCS utils/sys_util_linux.cc)
else()
    set(UTIL_SRCS utils/sys_util_linux.cc)
endif()

set(SRCS
    log_msg.cc
    log_handle.cc
    log_factory.cc
    formatter/default_formatter.cc
    sinks/console_sink.cc
    ${UTIL_SRCS}
)

add_library(turbo-log STATIC ${SRCS})
target_include_directories(turbo-log PUBLIC .)
target_link_libraries(turbo-log PUBLIC fmt::fmt)
