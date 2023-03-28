#ifndef SRC_UTIL_LOG_HPP
#define SRC_UTIL_LOG_HPP

#include <fmt/format.h>
#include <source_location>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <concurrentqueue.h>
#pragma clang diagnostic pop

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

#define MAKE_LOGGER(LEVEL)                                                     \
    template<class... T>                                                       \
    struct log##LEVEL                                                          \
    {                                                                          \
        log##LEVEL(                                                            \
            fmt::format_string<T...> fmt,                                      \
            T&&... args,                                                       \
            const std::source_location& location =                             \
                std::source_location::current()                                \
        ) noexcept                                                             \
        {                                                                      \
            using enum Level;                                                  \
            logFormatted(                                                      \
                LEVEL,                                                         \
                location,                                                      \
                fmt::vformat(fmt, fmt::make_format_args(args...))              \
            );                                                                 \
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
                std::source_location::current()                                \
        )                                                                      \
        {                                                                      \
            if (!condition)                                                    \
            {                                                                  \
                using enum Level;                                              \
                logFormatted(                                                  \
                    LEVEL,                                                     \
                    location,                                                  \
                    fmt::vformat(fmt, fmt::make_format_args(args...))          \
                );                                                             \
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
} // namespace util

#endif // SRC_UTIL_LOG_HPP
