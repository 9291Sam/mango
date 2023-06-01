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

        constexpr std::size_t line() const noexcept
        {
            return this->lineNumber;
        }

        constexpr const char* file() const noexcept
        {
            return this->fileName;
        }

        constexpr operator std::string () const noexcept
        {
            std::string raw_file_name {this->fileName};

            std::size_t index       = std::string::npos;
            std::size_t removalSize = 0;

            for (std::string_view str : FOLDER_IDENTIFIERS)
            {
                if (index != std::string::npos)
                {
                    break;
                }

                index       = raw_file_name.find(str);
                removalSize = str.size();
            }

            return raw_file_name.substr(index + 1);
        }

    private:
        uint_least32_t lineNumber;
        const char*    fileName;
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

    // template<class T..., Level l>
    // void adf();

    // template<class... T>
    // using logFatal<T, Fatal> = asd;

    // // global lambdas !

    // template<class... T>
    // auto logFatal = [] <

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

    MAKE_ASSERT(Trace, false) MAKE_ASSERT(Debug, false);
    MAKE_ASSERT(Log, false) MAKE_ASSERT(Warn, false);
    MAKE_ASSERT(Fatal, true);

    template<class... T>
    struct panic
    {
        [[noreturn]] panic(
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
