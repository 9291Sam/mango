#ifndef SRC_UTIL_MISC_HPP
#define SRC_UTIL_MISC_HPP

namespace util
{

    template<class T>
    constexpr T map(T x, T in_min, T in_max, T out_min, T out_max) noexcept
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

} // namespace util

#endif // SRC_UTIL_MISC_HPP
