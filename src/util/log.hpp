#ifndef SRC_UTIL_LOG_HPP
#define SRC_UTIL_LOG_HPP

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#pragma gcc diagnostic push
#pragma gcc diagnostic ignored "-Wstringop-overflow="
#include <fmt/format.h>
#pragma gcc diagnostic pop

#pragma clang diagnostic pop

#include "misc.hpp"
#include <string_view>
#include <utility>

namespace util
{
    static constexpr std::array<std::string_view, 2> FOLDER_IDENTIFIERS {
        "/src/", "/inc/"};

    class SourceLocation
    {
    public:
        static consteval SourceLocation current(
            const std::size_t line = __builtin_LINE(),
            const char* const file = __builtin_FILE()) noexcept
        {
            SourceLocation result {};
            result.lineNumber = line;
            result.fileName   = file;
            return result;
        }
    public:

        constexpr SourceLocation()  = default;
        constexpr ~SourceLocation() = default;

        constexpr SourceLocation(const SourceLocation&)             = default;
        constexpr SourceLocation(SourceLocation&&)                  = default;
        constexpr SourceLocation& operator= (const SourceLocation&) = default;
        constexpr SourceLocation& operator= (SourceLocation&&)      = default;

        [[nodiscard]] constexpr std::size_t line() const noexcept
        {
            return this->lineNumber;
        }

        [[nodiscard]] constexpr const char* file() const noexcept
        {
            return this->fileName;
        }

        constexpr explicit operator std::string () const noexcept
        {
            std::string raw_file_name {this->fileName};

            std::size_t index = std::string::npos;

            for (std::string_view str : FOLDER_IDENTIFIERS)
            {
                if (index != std::string::npos)
                {
                    break;
                }

                index = raw_file_name.find(str);
            }

            return fmt::format(
                "{}:{}", raw_file_name.substr(index + 1), this->lineNumber);
        }

    private:
        std::size_t lineNumber;
        const char* fileName;
    };

    enum class Level
    {
        Trace,
        Debug,
        Log,
        Warn,
        Fatal
    };

    void logFormatted(Level, const util::SourceLocation&, std::string);

/// Because C++ doesn't have partial template specification, this is the best we
/// can do
#define MAKE_LOGGER(LEVEL)                                                     \
    template<class... T>                                                       \
    struct log##LEVEL                                                          \
    {                                                                          \
        log##LEVEL(                                                            \
            fmt::format_string<T...> fmt,                                      \
            T&&... args,                                                       \
            const util::SourceLocation& location =                             \
                util::SourceLocation::current()) noexcept                      \
        {                                                                      \
            using enum Level;                                                  \
            logFormatted(                                                      \
                LEVEL,                                                         \
                location,                                                      \
                fmt::vformat(fmt, fmt::make_format_args(args...)));            \
        }                                                                      \
    };                                                                         \
    template<class... J>                                                       \
    log##LEVEL(fmt::format_string<J...>, J&&...)->log##LEVEL<J...>;

    MAKE_LOGGER(Trace) // NOLINT: We want implicit conversions
    MAKE_LOGGER(Debug) // NOLINT: We want implicit conversions
    MAKE_LOGGER(Log)   // NOLINT: We want implicit conversions
    MAKE_LOGGER(Warn)  // NOLINT: We want implicit conversions
    MAKE_LOGGER(Fatal) // NOLINT: We want implicit conversions

#define MAKE_ASSERT(LEVEL, THROW_ON_FAIL)                                      \
    template<class... T>                                                       \
    struct assert##LEVEL                                                       \
    {                                                                          \
        assert##LEVEL(                                                         \
            bool                     condition,                                \
            fmt::format_string<T...> fmt,                                      \
            T&&... args,                                                       \
            const util::SourceLocation& location =                             \
                util::SourceLocation::current())                               \
        {                                                                      \
            if (!condition)                                                    \
            {                                                                  \
                using enum Level;                                              \
                logFormatted(                                                  \
                    LEVEL,                                                     \
                    location,                                                  \
                    fmt::vformat(fmt, fmt::make_format_args(args...)));        \
                if constexpr (THROW_ON_FAIL)                                   \
                {                                                              \
                    throw std::runtime_error {                                 \
                        fmt::vformat(fmt, fmt::make_format_args(args...))};    \
                }                                                              \
            }                                                                  \
        }                                                                      \
    };                                                                         \
    template<class... J>                                                       \
    assert##LEVEL(bool, fmt::format_string<J...>, J&&...)                      \
        ->assert##LEVEL<J...>;

    MAKE_ASSERT(Trace, false) // NOLINT: We want implicit conversions
    MAKE_ASSERT(Debug, false) // NOLINT: We want implicit conversions
    MAKE_ASSERT(Log, false)   // NOLINT: We want implicit conversions
    MAKE_ASSERT(Warn, false)  // NOLINT: We want implicit conversions
    MAKE_ASSERT(Fatal, true)  // NOLINT: We want implicit conversions

    template<class... T>
    struct panic
    {
        [[noreturn]] panic( // NOLINT: implicit conversion
            fmt::format_string<T...> fmt,
            T&&... args,
            const util::SourceLocation& location =
                util::SourceLocation::current())
        {
            const std::string PanicString = fmt::format(
                "Thread panic! | Message: {}",
                fmt::vformat(fmt, fmt::make_format_args(args...)));

            using enum Level;
            logFormatted(Fatal, location, PanicString);
            throw std::runtime_error {PanicString};
            util::unreachable();
        }
    };
    template<class... J>
    panic(fmt::format_string<J...>, J&&...) -> panic<J...>;

    [[noreturn]] inline void
    todo(const util::SourceLocation& location = util::SourceLocation::current())
    {
        panic<>("Struck todo statement!", location);

        throw std::runtime_error {
            "Something has gone terribly wrong, panic has not thrown an "
            "exception!"};
    }
} // namespace util

#endif // SRC_UTIL_LOG_HPP
