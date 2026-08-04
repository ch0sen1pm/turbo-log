#pragma once

#include "log_common.h"
#include "log_factory.h"
#include "log_variadic_handle.h"


#define EXT_LOGGER_INIT(handle) \
    logger::LogFactory::Instance().SetLogHandle(handle)

#define LOGGER_CALL(handle, level, ...)                             \
    if (handle) {                                                   \
        (handle)->Log(                                              \
            logger::SourceLocation{__FILE__, __LINE__, __FUNCTION__},\
            level,                                                  \
            __VA_ARGS__);                                           \
    }
    
#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_TRACE
#define LOG_LOGGER_TRACE(handle, ...)   LOGGER_CALL(handle, logger::LogLevel::kTrace, __VA_ARGS__)
#define EXT_LOG_TRACE(...)              LOG_LOGGER_TRACE(logger::LogFactory::Instance().GetLogHandle(), __VA_ARGS__)
#else
#define LOG_LOGGER_TRACE(handle, ...)   (void)0
#define EXT_LOG_TRACE(...)              (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_DEBUG
#define LOG_LOGGER_DEBUG(handle, ...)   LOGGER_CALL(handle, logger::LogLevel::kDebug, __VA_ARGS__)
#define EXT_LOG_DEBUG(...)              LOG_LOGGER_DEBUG(logger::LogFactory::Instance().GetLogHandle(), __VA_ARGS__)
#else
#define LOG_LOGGER_DEBUG(handle, ...)   (void)0
#define EXT_LOG_DEBUG(...)              (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_INFO
#define LOG_LOGGER_INFO(handle, ...)    LOGGER_CALL(handle, logger::LogLevel::kInfo, __VA_ARGS__)
#define EXT_LOG_INFO(...)               LOG_LOGGER_INFO(logger::LogFactory::Instance().GetLogHandle(), __VA_ARGS__)
#else
#define LOG_LOGGER_INFO(handle, ...)    (void)0
#define EXT_LOG_INFO(...)               (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_WARN
#define LOG_LOGGER_WARN(handle, ...)    LOGGER_CALL(handle, logger::LogLevel::kWarn, __VA_ARGS__)
#define EXT_LOG_WARN(...)               LOG_LOGGER_WARN(logger::LogFactory::Instance().GetLogHandle(), __VA_ARGS__)
#else
#define LOG_LOGGER_WARN(handle, ...)    (void)0
#define EXT_LOG_WARN(...)               (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_ERROR
#define LOG_LOGGER_ERROR(handle, ...)   LOGGER_CALL(handle, logger::LogLevel::kError, __VA_ARGS__)
#define EXT_LOG_ERROR(...)              LOG_LOGGER_ERROR(logger::LogFactory::Instance().GetLogHandle(), __VA_ARGS__)
#else
#define LOG_LOGGER_ERROR(handle, ...)   (void)0
#define EXT_LOG_ERROR(...)              (void)0
#endif

#if LOGGER_ACTIVE_LEVEL <= LOGGER_LEVEL_CRITICAL
#define LOG_LOGGER_CRITICAL(handle, ...) LOGGER_CALL(handle, logger::LogLevel::kFatal, __VA_ARGS__)
#define EXT_LOG_CRITICAL(...)            LOG_LOGGER_CRITICAL(logger::LogFactory::Instance().GetLogHandle(), __VA_ARGS__)
#else
#define LOG_LOGGER_CRITICAL(handle, ...) (void)0
#define EXT_LOG_CRITICAL(...)            (void)0
#endif