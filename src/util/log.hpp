#include <array>
#include <format>
#include <iostream>
#include <source_location>
#include <string_view>

namespace log
{

    class AsyncStdoutLogger
    {
        void logMessage(std::string);
    };

    static AsyncStdoutLogger LOGGER;

    enum class Level
    {
        Trace,
        Debug,
        Log,
        Warn,
        Fatal
    };

    static constexpr std::array<std::string_view, 2> FOLDER_IDENTIFIERS {
        "/src/", "/inc/"};

    // We dont talk about this "function" which is actually a struct because
    // deduction guides
    template<class... T>
    struct logMessage
    {
        logMessage(
            std::_Fmt_string<T...> fmt,
            T&&... args,
            const std::source_location& location =
                std::source_location::current()
        )
        {
            auto currentTime = std::chrono::system_clock::now();

            std::cout << fmt::format(
                "[{0}] [{1}:{2}:{3}] [{4}] {5}\n",
                [&] { // 0
                    std::string time = fmt::format(
                        "{:%b %m/%d/%Y} {:%I:%M:%S %p}",
                        currentTime,
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            currentTime.time_since_epoch()
                        )
                    );
                    return time.substr(0, 27).append(":").append(time.substr(27)
                    );
                }(), // 0
                [&] { // 1
                    std::string raw_file_name = location.file_name();

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
                }(), // 1
                location.line(), // 2
                location.column(), // 3
                static_cast<std::string>("Trace"), // 4
                std::vformat(fmt, std::make_format_args(args...)) // 5
            );
        }
    };
    template<class... T>
    logMessage(std::_Basic_format_string<T...>, T&&...) -> logMessage<T...>;

    // template<class... T>
    // using trace = logMessage<Level::Trace, T...>;

} // namespace log
