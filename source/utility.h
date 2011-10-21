#ifndef ROADS_UTILITY_H
#define ROADS_UTILITY_H

namespace roads {
    template <typename T, size_t N>
    char(&(countof_helper)(T(&)[N]))[N];
#define countof(x) sizeof(::roads::countof_helper(x))
}

#endif // ROADS_UTILITY_H

