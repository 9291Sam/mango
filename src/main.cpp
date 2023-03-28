#include <array>
#include <concurrentqueue.h>
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

class AsyncStdoutLogger
{
public:
    AsyncStdoutLogger()
        : should_thread_close {false}
    {
        this->worker_thread = std::thread {
            [this]
            {
                std::string temporary_string {"INVALID MESSAGE"};

                // Main loop
                while (!this->should_thread_close->load())
                {
                    if (this->message_queue.try_dequeue(temporary_string))
                    {
                        std::cout << temporary_string;
                    }
                }

                // Cleanup loop
                while (this->message_queue.try_dequeue(temporary_string))
                {
                    std::cout << temporary_string;
                }
            }};
    }

    ~AsyncStdoutLogger()
    {
        this->should_thread_close->store(true);

        this->worker_thread.join();
    }

    AsyncStdoutLogger(const AsyncStdoutLogger&)             = delete;
    AsyncStdoutLogger(AsyncStdoutLogger&&)                  = delete;
    AsyncStdoutLogger& operator= (const AsyncStdoutLogger&) = delete;
    AsyncStdoutLogger& operator= (AsyncStdoutLogger&&)      = delete;

    void sendMessage(std::string&& string)
    {
        if (!this->message_queue.enqueue(std::move(string)))
        {
            std::cerr << "Failed to send message | memory allocation failed"
                      << string;
        };
    }

private:
    std::thread                              worker_thread;
    std::shared_ptr<std::atomic<bool>>       should_thread_close;
    moodycamel::ConcurrentQueue<std::string> message_queue;
};

static AsyncStdoutLogger logger {};

void logFormatted(Level l, const std::source_location& loc, std::string msg)
{
    logger.sendMessage(fmt::format(
        "[{0}] [{1} @ {2}:{3}:{4}] [{5}] {6}\n",
        [&]
        {
            auto const time = std::chrono::current_zone()->to_local(
                std::chrono::system_clock::now()
            );
            std::string timeString =
                std::format("{:%b %m/%d/%Y %I:%M:%S %p}", time);

            return timeString.substr(0, 27).append(":").append(
                timeString.substr(28)
            );
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

int main()
{
    logTrace("asdf");
    logDebug("asdf");
    logLog("asdf");
    logWarn("asdf");
    logFatal("asdf");
}
