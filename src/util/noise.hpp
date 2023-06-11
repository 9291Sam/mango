#ifndef SRC_UTIL_NOISE_HPP
#define SRC_UTIL_NOISE_HPP

#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numbers>
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

    class LCG
    {
    public:

        constexpr explicit LCG(std::uint64_t seed_)
            : seed(seed_)
            , multiplier(6364136223846793005ULL)
            , increment(1442695040888963407ULL)
            , modulus(18436744073709551616ULL)
        {
            std::ignore = this->next();
            std::ignore = this->next();
            std::ignore = this->next();
        }
        constexpr ~LCG() = default;

        constexpr explicit LCG(const LCG&)    = default;
        constexpr LCG(LCG&&)                  = default;
        constexpr LCG& operator= (const LCG&) = delete;
        constexpr LCG& operator= (LCG&&)      = default;

        constexpr std::uint64_t next()
        {
            constexpr std::uint64_t width {8 * sizeof(std::uint64_t)};
            constexpr std::uint64_t offset {width / 2};

            const std::uint64_t prev = this->seed;

            this->seed = (this->multiplier * this->seed + this->increment)
                       % this->modulus;

            std::uint64_t output =
                (this->multiplier * this->seed) % this->modulus;

            output ^= prev << offset | prev >> (width - offset);

            return output;
        }

    private:
        std::uint64_t seed;
        std::uint64_t multiplier;
        std::uint64_t increment;
        std::uint64_t modulus;
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

    typedef struct
    {
        float x, y;
    } vector2;

    /* Create pseudorandom direction vector
     */
    constexpr inline vector2 randomGradient(int ix, int iy)
    {
        // No precomputed gradients mean this works for any number of grid
        // coordinates
        // const unsigned w = 8 * sizeof(unsigned);
        // const unsigned s = w / 2; // rotation width
        // unsigned       a = ix, b = iy;
        // a *= 3284157443;
        // b ^= a << s | a >> (w - s);
        // b *= 1911520717;
        // a ^= b << s | b >> (w - s);
        // a *= 2048419325;
        // float   random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

        std::uint64_t workingSeed {
            std::bit_cast<std::uint64_t>(78234748926789234)};

        util::hashCombine(workingSeed, ix);
        util::hashCombine(workingSeed, iy);

        LCG engine {workingSeed};

        float random = util::map<float>(
            engine.next(),
            static_cast<float>(std::numeric_limits<std::uint64_t>::min()),
            static_cast<float>(std::numeric_limits<std::uint64_t>::max()),
            0.0f,
            std::numbers::pi_v<float> * 2);
        vector2 v;
        v.x = cos(random);
        v.y = sin(random);
        return v;
    }

    // Computes the dot product of the distance and gradient vectors.
    constexpr inline float dotGridGradient(int ix, int iy, float x, float y)
    {
        // Get gradient from integer coordinates
        vector2 gradient = randomGradient(ix, iy);

        // Compute the distance vector
        float dx = x - (float)ix;
        float dy = y - (float)iy;

        // Compute the dot-product
        return (dx * gradient.x + dy * gradient.y);
    }

    // Compute Perlin noise at coordinates x, y
    constexpr inline float perlin(float x, float y)
    {
        // Determine grid cell coordinates
        int x0 = (int)floor(x);
        int x1 = x0 + 1;
        int y0 = (int)floor(y);
        int y1 = y0 + 1;

        // Determine interpolation weights
        // Could also use higher order polynomial/s-curve here
        float sx = x - (float)x0;
        float sy = y - (float)y0;

        // Interpolate between grid point gradients
        float n0, n1, ix0, ix1, value;

        n0  = dotGridGradient(x0, y0, x, y);
        n1  = dotGridGradient(x1, y0, x, y);
        ix0 = quarticInterpolate(n0, n1, sx);

        n0  = dotGridGradient(x0, y1, x, y);
        n1  = dotGridGradient(x1, y1, x, y);
        ix1 = quarticInterpolate(n0, n1, sx);

        value = quarticInterpolate(ix0, ix1, sy);
        return value; // Will return in range -1 to 1. To make it in range 0 to
                      // 1, multiply by 0.5 and add 0.5
    }
    // constexpr inline std::int64_t
    // generatePerlinNoise2D(std::size_t seed, std::int64_t x, std::int64_t y)
    // {
    //     // generate vectors around the position
    //     // smooth
    //     //
    // }

} // namespace util

#endif // SRC_UTIL_NOISE_HPP