#pragma once

#include <string.h>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#include <fmt/format.h>

#define LOGGER_LEVEL_TRACE 0
#define LOGGER_LEVEL_DEBUG 1
#define LOGGER_LEVEL_INFO 2
#define LOGGER_LEVEL_WARN 3
#define LOGGER_LEVEL_ERROR 4
#define LOGGER_LEVEL_CRITICAL 5
#define LOGGER_LEVEL_OFF 6

namespace logger {
using StringView = std::string_view;
using MemoryBuf = std::string;

template <typename... Args>
using FormatString = fmt::format_string<Args...>;

template <typename T>
using remove_cvref_t = typename std::remove_cv<
                            typename std::remove_reference<T>::type
                        >::type;

template <typename T, typename Char = char>
struct is_convertible_to_basic_format_string
    : std::integral_constant<bool,
                             std::is_convertible<T, fmt::basic_string_view<Char>>::value || 
                             std::is_same<remove_cvref_t<T>, fmt::runtime_format_string<Char>>::value> {};

template <typename T>
struct is_convertible_to_any_format_string
    : std::integral_constant<bool,
                             is_convertible_to_basic_format_string<T, char>::value ||
                             is_convertible_to_basic_format_string<T, wchar_t>::value> {};

enum class LogLevel {
    kTrace = LOGGER_LEVEL_TRACE,       // = 0
    kDebug = LOGGER_LEVEL_DEBUG,       // = 1
    kInfo  = LOGGER_LEVEL_INFO,        // = 2
    kWarn  = LOGGER_LEVEL_WARN,        // = 3
    kError = LOGGER_LEVEL_ERROR,       // = 4
    kFatal = LOGGER_LEVEL_CRITICAL,    // = 5
    kOff   = LOGGER_LEVEL_OFF          // = 6
};

#define LOGGER_ACTIVE_LEVEL LOGGER_LEVEL_TRACE

struct SourceLocation {
    constexpr SourceLocation() = default;

    SourceLocation(StringView file_name_in,
                   int32_t    line_in,
                   StringView func_name_in)
        : file_name{file_name_in},
          line{line_in},
          func_name{func_name_in}
    {
        if (!file_name.empty()) {
            size_t pos = file_name.rfind('/');
            if (pos != StringView::npos) {
                file_name = file_name.substr(pos + 1);
            } else {
               pos = file_name.rfind('\\');
               if (pos != StringView::npos) {
                file_name = file_name.substr(pos + 1);
               } 
            }
        }
    }


    StringView file_name;
    int32_t line{0};
    StringView func_name;
};

} // namespace logger
