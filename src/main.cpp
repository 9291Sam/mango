#include <array>
#include <concurrentqueue.h>
#include <ctime>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <format>
#include <iostream>
#include <source_location>

enum class Level
{
    Trace,
    Debug,
    Log,
    Warn,
    Fatal
};

const char* const levelAsString(Level l)
{
    using enum Level;
    switch (l)
    {
    case Trace:
        return "Trace";
    case Debug:
        return "Debug";
    case Log:
        return "Log";
    case Warn:
        return "Warn";
    case Fatal:
        return "Fatal";
    }
}

static constexpr std::array<std::string_view, 2> FOLDER_IDENTIFIERS {
    "/src/", "/inc/"};

static moodycamel::ConcurrentQueue<std::string> stdoutStringQueue {};

void logFormatted(Level l, const std::source_location& loc, std::string msg)
{
    stdoutStringQueue.enqueue(fmt::format(
        "[{0}] [{1} @ {2}:{3}:{4}] [{5}] {6}\n",
        [&]
        {
            std::string workingString {31, ' '};

            workingString = fmt::format(
                "{:0%b %m/%d/%Y %I:%M:}{:%S}",
                fmt::localtime(std::time(nullptr)),
                std::chrono::system_clock::now()
            );

            workingString.at(workingString.size() - 7) = ':';
            workingString.insert(workingString.size() - 3, ":");

            return workingString;
        }(), // 0
        loc.function_name(), // 1
        [&]
        {
            std::string raw_file_name = loc.file_name();

            std::size_t index = std::string::npos;

            for (std::string_view str : FOLDER_IDENTIFIERS)
            {
                if (index != std::string::npos)
                {
                    break;
                }

                index = raw_file_name.find(str);
            }

            return raw_file_name.substr(index + 1);
        }(), // 2
        loc.line(), // 3
        loc.column(), // 4
        levelAsString(l), // 5
        msg
    ));
}

#define MAKE_LOGGER(LEVEL)                                                     \
    template<class... T>                                                       \
    struct log##LEVEL                                                          \
    {                                                                          \
        constexpr log##LEVEL(                                                  \
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

// class AsyncStdoutLogger
// {
// public:
//     AsyncStdoutLogger(const moodycamel::ConcurrentQueue<std::string>& queue)
//     {}

// private:
//     std::thread thread;

// }

int main()
{
    logLog("{}", "asdf");

    std::string str = std::string {"invalid"};

    stdoutStringQueue.try_dequeue(str);

    std::cout << str << std::endl;
}
