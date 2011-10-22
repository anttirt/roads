#ifndef ROADS_UTILITY_H
#define ROADS_UTILITY_H

namespace roads {
    template <typename T, size_t N>
    char(&(countof_helper)(T(&)[N]))[N];
#define countof(x) sizeof(::roads::countof_helper(x))

	typedef uint16_t rgb;

    constexpr rgb make_rgb(uint8_t r, uint8_t g, uint8_t b) {
        return uint16_t(r) | (uint16_t(g) << 5) | (uint16_t(b) << 10);
    }

    constexpr int16_t float_to_v10(float f) {
        return (f > .998)
            ? 0x1FF
            : ((int16_t)(f*(1<<9)));
    }

    constexpr int16_t clamp(int16_t val, int16_t min, int16_t max) {
        return val < min ? min : val > max ? max : val;
    }

}

#endif // ROADS_UTILITY_H

