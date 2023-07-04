#include "log.hpp"
#include <ctime>
#include <iostream>
#include <string_view>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <concurrentqueue.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#pragma clang diagnostic pop

const char* levelAsString(util::Level l)
{
    using enum util::Level;

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
                // util::logTrace("Logger's destructor called");

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

void util::logFormatted(Level l, const SourceLocation& loc, std::string msg)
{
    logger.sendMessage(fmt::format(
        "[{0}] [{1}] [{2}] {3}\n",
        [&]
        {
            std::string workingString {31, ' '};

            workingString = fmt::format(
                "{:0%b %m/%d/%Y %I:%M}:{:%S}",
                fmt::localtime(std::time(nullptr)),
                std::chrono::system_clock::now());

            workingString.erase(30, std::string::npos);

            workingString.at(workingString.size() - 7) = ':';
            workingString.insert(workingString.size() - 3, ":");

            return workingString;
        }(),                           // 0
        static_cast<std::string>(loc), // 1
        levelAsString(l),              // 2
        msg                            // 3
        ));
}
