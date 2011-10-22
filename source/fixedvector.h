#if defined(DSR_FIXED16_H) && defined(DSR_VECTOR_H_INCLUDED)

#ifndef ROADS_FIXEDVECTOR_H
#define ROADS_FIXEDVECTOR_H

#include "utility.h"

namespace roads {
	typedef vector<f16, 2> vector2f16;
	typedef vector<f16, 3> vector3f16;

	inline constexpr vector2f16 vec(f16 x, f16 y)
	{
        return vector2f16 { x, y };
	}
	inline constexpr vector3f16 vec(f16 x, f16 y, f16 z)
	{
		return vector3f16 { x, y, z };
	}
    inline constexpr vector3f16 vec(vector2f16 xy, f16 z)
    {
        return vector3f16 { xy.x, xy.y, z };
    }

    // Convert three 4.12 fixed point numbers to 1.9 and pack them into a 32-bit
    // value, leaving 2 bits unused. This format is used when passing vertex
    // normals to the graphics hardware.
    constexpr uint32_t normal_pack(vector3f16 v) {
        return ((uint16_t(clamp(v.x.raw_value, -4096, 4095) >> 3) & 0x3FF))
             | ((uint16_t(clamp(v.y.raw_value, -4096, 4095) >> 3) & 0x3FF) << 10)
             | ((uint16_t(clamp(v.z.raw_value, -4096, 4095) >> 3) & 0x3FF) << 20);
    }
}

#endif // ROADS_FIXEDVECTOR_H

#endif // fixed && vector

