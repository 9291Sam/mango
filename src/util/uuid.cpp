#include "uuid.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <random>

namespace util
{
    UUID::UUID()
    {
        static std::atomic<std::uint64_t> id {std::random_device {}()};

        this->timestamp = crc64(static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count()));
        this->id_number = crc64(id++);
    }

    UUID::operator std::string () const
    {
        return fmt::format(
            "{:08X}~{:08X}~{:08X}~{:08X}",
            this->timestamp >> 32,
            this->timestamp & 0xFFFF'FFFF,
            this->id_number >> 32,
            this->id_number & 0xFFFF'FFFF);
    }
} // namespace util