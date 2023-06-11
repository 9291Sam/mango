#ifndef SRC_UTIL_NOISE_HPP
#define SRC_UTIL_NOISE_HPP

#include "util/misc.hpp"
#include "vector.hpp"
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <random>
#include <tuple>
#include <util/misc.hpp>

///
/// This entire implementation is unceremoniously sz`tolen from the wikipedia
/// page on perlin noise
/// https://en.wikipedia.org/w/index.php?title=Perlin_noise&oldid=1148235423
///

namespace util
{
    // TODO: N dimensional noise

    class FastMCG
    {
    public:

        constexpr explicit FastMCG(std::uint64_t seed_)
            : seed {seed_}
            , multiplier {6364136223846793005ULL}
            , increment {1442695040888963407ULL}
        {
            std::ignore = this->next();
        }
        constexpr ~FastMCG() = default;

        constexpr explicit FastMCG(const FastMCG&)    = default;
        constexpr FastMCG(FastMCG&&)                  = default;
        constexpr FastMCG& operator= (const FastMCG&) = delete;
        constexpr FastMCG& operator= (FastMCG&&)      = default;

        constexpr std::uint64_t next()
        {
            constexpr std::uint64_t width {8 * sizeof(std::uint64_t)};
            constexpr std::uint64_t offset {width / 2};

            const std::uint64_t prev = this->seed;

            this->seed = (this->multiplier * this->seed + this->increment);

            std::uint64_t output = this->seed;

            output ^= prev << offset | prev >> (width - offset);

            return output;
        }

        template<class T>
        constexpr T nextInRange(T min, T max)
            requires (std::convertible_to<T, long double>)
        {
            return static_cast<T>(util::map<long double>(
                this->next(),
                static_cast<long double>(
                    std::numeric_limits<std::uint64_t>::min()),
                static_cast<long double>(
                    std::numeric_limits<std::uint64_t>::max()),
                static_cast<long double>(min),
                static_cast<long double>(max)));
        }

    private:
        std::uint64_t seed;
        std::uint64_t multiplier;
        std::uint64_t increment;
    };

    /// Interpolates between [value1, rightBound] given a weight [0.0, 1.0]
    /// along the range
    template<std::floating_point T>
    constexpr inline T quarticInterpolate(T leftBound, T rightBound, T weight)
    {
        constexpr T Six {static_cast<T>(6.0)};
        constexpr T Fifteen {static_cast<T>(15.0)};
        constexpr T Ten {static_cast<T>(10.0)};

        return (rightBound - leftBound)
                 * ((weight * (weight * Six - Fifteen) + Ten) * weight * weight
                    * weight)
             + leftBound;
    }

    template<std::size_t N> // TODO: replace with some AbsolutePosition
    constexpr inline Vec2 randomGradient(Vector<std::int64_t, N> vector)
    {
        std::uint64_t workingSeed {
            std::bit_cast<std::uint64_t>(78234748926789234)};

        for (std::int64_t i : vector.data)
        {
            util::hashCombine(workingSeed, i);
        }

        FastMCG engine {workingSeed};

        float random =
            engine.nextInRange<float>(0.0f, std::numbers::pi_v<float> * 2);

        // gcem is ~3-5x slower than std, try and avoid it
        if consteval
        {
            return Vec2 {gcem::cos(random), gcem::sin(random)};
        }
        else
        {
            return Vec2 {std::cos(random), std::sin(random)};
        }
    }

    template<std::size_t N>
    constexpr inline float dotGridGradient(
        Vector<std::int64_t, N> position,
        Vector<float, N>        perlinGridGranularity)
    {
        // Get gradient from integer coordinates
        const Vec2 gradient = randomGradient(position);

        // Compute the distance vector
        const Vector<float, N> offset {
            perlinGridGranularity - static_cast<Vector<float, N>>(position)};

        return offset.dot(gradient);
    }

    // Compute Perlin noise at coordinates x, y
    // TODO: replace with some AbsolutePosition with an int64 + a float
    // TODO: add seeds
    constexpr inline float perlin(Vec2 vector) // TODO: make generic
    {
        const Vector<std::int64_t, 2> BottomLeftGrid {
            static_cast<std::int64_t>(std::floor(vector.x())),
            static_cast<std::int64_t>(std::floor(vector.y())),
        };

        const Vector<std::int64_t, 2> TopRightGrid {BottomLeftGrid + 1};

        const Vector<std::int64_t, 2> BottomRightGrid {
            BottomLeftGrid.x(), TopRightGrid.y()};

        const Vector<std::int64_t, 2> TopLeftGrid {
            TopRightGrid.x(), BottomLeftGrid.y()};

        const Vec2 OffsetIntoGrid {vector - static_cast<Vec2>(BottomLeftGrid)};

        const float LeftGradient = quarticInterpolate(
            dotGridGradient(BottomLeftGrid, vector),
            dotGridGradient(TopLeftGrid, vector),
            OffsetIntoGrid.x());

        const float RightGradient = quarticInterpolate(
            dotGridGradient(BottomRightGrid, vector),
            dotGridGradient(TopRightGrid, vector),
            OffsetIntoGrid.x());

        return quarticInterpolate(
            LeftGradient, RightGradient, OffsetIntoGrid.y());
    }

} // namespace util

#endif // SRC_UTIL_NOISE_HPP