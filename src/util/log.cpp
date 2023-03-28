#include "log.hpp"
#include <ctime>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <concurrentqueue.h>
#pragma clang diagnostic pop

namespace util
{

    static constexpr std::array<std::string_view, 2> FOLDER_IDENTIFIERS {
        "/src/", "/inc/"};

    const char* levelAsString(Level l)
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

        throw std::runtime_error {"unreachable!"};
    }

    class AsyncStdoutLogger
    {
    public:
        AsyncStdoutLogger()
            : should_thread_close {std::make_shared<std::atomic_bool>(false)}
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
                    util::logTrace("Logger's destructor called");

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
            }
        }

    private:
        std::thread                              worker_thread;
        std::shared_ptr<std::atomic<bool>>       should_thread_close;
        moodycamel::ConcurrentQueue<std::string> message_queue;
    };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
    static AsyncStdoutLogger logger {};
#pragma clang diagnostic pop

    void logFormatted(Level l, const std::source_location& loc, std::string msg)
    {
        logger.sendMessage(fmt::format(
            "[{0}] [{1} @ {2}:{3}:{4}] [{5}] {6}\n",
            [&]
            {
                std::string workingString {31, ' '};

                workingString = fmt::format(
                    "{:0%b %m/%d/%Y %I:%M}:{:%S}",
                    fmt::localtime(std::time(nullptr)),
                    std::chrono::system_clock::now()
                );

                workingString.at(workingString.size() - 8) = ':';
                workingString.pop_back();
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

} // namespace util
