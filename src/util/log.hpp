#ifndef SRC_UTIL_LOG_HPP
#define SRC_UTIL_LOG_HPP

#pragma gcc diagnostic push
#pragma gcc diagnostic ignored "-Wstringop-overflow="
#include <fmt/format.h>
#pragma gcc diagnostic pop

#include <source_location>
#include <utility>

namespace util
{
    enum class Level
    {
        Trace,
        Debug,
        Log,
        Warn,
        Fatal
    };

    void logFormatted(Level, const std::source_location&, std::string);

/// Because C++ doesn't have partial template specification, this is the best we
/// can do
#define MAKE_LOGGER(LEVEL)                                                     \
    template<class... T>                                                       \
    struct log##LEVEL                                                          \
    {                                                                          \
        log##LEVEL(                                                            \
            fmt::format_string<T...> fmt,                                      \
            T&&... args,                                                       \
            const std::source_location& location =                             \
                std::source_location::current()) noexcept                      \
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

    MAKE_LOGGER(Trace)
    MAKE_LOGGER(Debug)
    MAKE_LOGGER(Log)
    MAKE_LOGGER(Warn)
    MAKE_LOGGER(Fatal)

#define MAKE_ASSERT(LEVEL, THROW_ON_FAIL)                                      \
    template<class... T>                                                       \
    struct assert##LEVEL                                                       \
    {                                                                          \
        assert##LEVEL(                                                         \
            bool                     condition,                                \
            fmt::format_string<T...> fmt,                                      \
            T&&... args,                                                       \
            const std::source_location& location =                             \
                std::source_location::current())                               \
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

    MAKE_ASSERT(Trace, false)
    MAKE_ASSERT(Debug, false)
    MAKE_ASSERT(Log, false)
    MAKE_ASSERT(Warn, false)
    MAKE_ASSERT(Fatal, true)

    template<class... T>
    struct panic
    {
        [[noreturn]] panic(
            fmt::format_string<T...> fmt,
            T&&... args,
            const std::source_location& location =
                std::source_location::current())
        {
            const std::string PanicString = fmt::format(
                "Thread panic! | Message: {}",
                fmt::vformat(fmt, fmt::make_format_args(args...)));

            using enum Level;
            logFormatted(Fatal, location, PanicString);
            throw std::runtime_error {PanicString};
            std::unreachable();
        }
    };
    template<class... J>
    panic(fmt::format_string<J...>, J&&...) -> panic<J...>;

    [[noreturn]] inline void
    todo(const std::source_location& location = std::source_location::current())
    {
        panic<>("Struck todo statement!", location);

        throw std::runtime_error {
            "Something has gone terribly wrong, panic has not thrown an "
            "exception!"};
    }
} // namespace util

#endif // SRC_UTIL_LOG_HPP
