#ifndef ROADS_GEOMETRY_H
#define ROADS_GEOMETRY_H

#include "vector.h"

namespace roads {
    namespace geometry {
        typedef vector<double, 3> v3d;

        constexpr int16_t float_to_v10(float f) {
            return (f > .998)
                ? 0x1FF
                : ((int16_t)(f*(1<<9)));
        }

        constexpr uint32_t normal_pack(v3d a) {
            return
                ((float_to_v10(a.x) & 0x3FF)) |
                ((float_to_v10(a.y) & 0x3FF) << 10) |
                ((float_to_v10(a.z) & 0x3FF) << 20);
        }

        constexpr int16_t clamp(int16_t val, int16_t min, int16_t max) {
            return val < min ? min : val > max ? max : val;
        }

        constexpr uint32_t normal_pack(vector3 v) {
            return ((uint16_t(clamp(v.x.raw_value, -4096, 4095) >> 3) & 0x3FF))
                 | ((uint16_t(clamp(v.y.raw_value, -4096, 4095) >> 3) & 0x3FF) << 10)
                 | ((uint16_t(clamp(v.z.raw_value, -4096, 4095) >> 3) & 0x3FF) << 20);
        }

        namespace draw {
            constexpr f16 block_size = 1.f / 32.f;
        }

        namespace tunnel {
            constexpr double block_size = draw::block_size.to_float();
            constexpr double tile_size = block_size / 5.;

            // cos(30deg)
            constexpr double cos30 = 0.86602540378443864676372317075294;

            // half of a unit circle described
            // in seven points clockwise from (-1, 0) to (1, 0)
            // translated by (1,0,0) and scaled to fit a block,
            // possibly on top of a tile
            constexpr v3d circle_scale = v3d(0.5*block_size,block_size - tile_size,block_size);
            constexpr v3d unitx = v3d::unitx();
            constexpr v3d half_circle[] = {
                (v3d(     -1,      0, 0) + unitx).scale(circle_scale),
                (v3d( -cos30,    0.5, 0) + unitx).scale(circle_scale),
                (v3d(    0.5,  cos30, 0) + unitx).scale(circle_scale),
                (v3d(      0,      1, 0) + unitx).scale(circle_scale),
                (v3d(    0.5,  cos30, 0) + unitx).scale(circle_scale),
                (v3d(  cos30,    0.5, 0) + unitx).scale(circle_scale),
                (v3d(      1,      0, 0) + unitx).scale(circle_scale)
            };
            constexpr v3d tile = v3d(0, tile_size, 0);
            constexpr v3d inner_scale = v3d(5.f / 6.f, 5.f / 6.f, 5.f / 6.f);

            constexpr vector3 outer[] = {
                (half_circle[0] + tile).convert<f16>(),
                (half_circle[1] + tile).convert<f16>(),
                (half_circle[2] + tile).convert<f16>(),
                (half_circle[3] + tile).convert<f16>(),
                (half_circle[4] + tile).convert<f16>(),
                (half_circle[5] + tile).convert<f16>(),
                (half_circle[6] + tile).convert<f16>()
            };

            constexpr vector3 inner[] = {
                (half_circle[0].scale(inner_scale) + tile).convert<f16>(),
                (half_circle[1].scale(inner_scale) + tile).convert<f16>(),
                (half_circle[2].scale(inner_scale) + tile).convert<f16>(),
                (half_circle[3].scale(inner_scale) + tile).convert<f16>(),
                (half_circle[4].scale(inner_scale) + tile).convert<f16>(),
                (half_circle[5].scale(inner_scale) + tile).convert<f16>(),
                (half_circle[6].scale(inner_scale) + tile).convert<f16>()
            };

            constexpr f16 shell_height = block_size - tile_size;

            constexpr int32_t normals[] = {
                normal_pack(half_circle[0].normalized()),
                normal_pack(half_circle[1].normalized()),
                normal_pack(half_circle[2].normalized()),
                normal_pack(half_circle[3].normalized()),
                normal_pack(half_circle[4].normalized()),
                normal_pack(half_circle[5].normalized()),
                normal_pack(half_circle[6].normalized())
            };
        }
    };
}

#endif

