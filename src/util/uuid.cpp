#include "uuid.hpp"
#include <atomic>
#include <chrono>
#include <random>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <fmt/format.h>
#pragma clang diagnostic pop

util::UUID::UUID()
{
    static std::atomic<std::uint64_t> id {std::random_device {}()};

    this->timestamp = crc64(static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count()));
    this->id_number = crc64(id++);
}

util::UUID::operator std::string () const
{
    return fmt::format(
        "{:08X}~{:08X}~{:08X}~{:08X}",
        this->timestamp >> 32,
        this->timestamp & 0xFFFF'FFFF,
        this->id_number >> 32,
        this->id_number & 0xFFFF'FFFF);
}
std::strong_ordering util::UUID::operator<=> (const UUID& other) const
{
#define EMIT_ORDERING(field)                                                   \
    if (this->field <=> other.field != std::strong_ordering::equivalent)       \
    {                                                                          \
        return this->field <=> other.field;                                    \
    }

    EMIT_ORDERING(timestamp)
    EMIT_ORDERING(id_number)

    return std::strong_ordering::equivalent;
}
