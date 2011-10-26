#ifndef ROADS_UTILITY_H
#define ROADS_UTILITY_H

#include <type_traits>
#include <cstddef>

namespace roads {
    template <typename T, size_t N>
    char(&(countof_helper)(T(&)[N]))[N];
#define countof(x) sizeof(::roads::countof_helper(x))

	typedef uint16_t rgb;

    template <typename T>
    constexpr typename std::enable_if<std::is_integral<T>::value, rgb>::type make_rgb(T r, T g, T b) {
        return uint16_t(r) | (uint16_t(g) << 5) | (uint16_t(b) << 10);
    }

    template <typename T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, rgb>::type make_rgb(T r, T g, T b) {
        return uint16_t(r * 31) | (uint16_t(g * 31) << 5) | (uint16_t(b * 31) << 10);
    }

    constexpr int16_t float_to_v10(float f) {
        return (f > .998)
            ? 0x1FF
            : ((int16_t)(f*(1<<9)));
    }

    constexpr int16_t clamp(int16_t val, int16_t min, int16_t max) {
        return val < min ? min : val > max ? max : val;
    }

    enum class dimension {
        x, y, z
    };
}

#endif // ROADS_UTILITY_H

