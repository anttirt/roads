#include "cell.h"

#include "disp_writer.h"
#include "vector.h"
#include "fixed16.h"
#include "geometry.h"

#define MAKE_GRADIENT(MAKER) \
    MAKER( 0), MAKER( 1), MAKER( 2), MAKER( 3), \
    MAKER( 4), MAKER( 5), MAKER( 6), MAKER( 7), \
    MAKER( 8), MAKER( 9), MAKER(10), MAKER(11), \
    MAKER(12), MAKER(13), MAKER(14), MAKER(15), \
    MAKER(16), MAKER(17), MAKER(18), MAKER(19), \
    MAKER(20), MAKER(21), MAKER(22), MAKER(23), \
    MAKER(24), MAKER(25), MAKER(26), MAKER(27), \
    MAKER(28), MAKER(29), MAKER(30), MAKER(31)

#define MAKE_RED(R) make_rgb(R, 0, 0)
#define MAKE_GREEN(G) make_rgb(0, G, 0)
#define MAKE_BLUE(B) make_rgb(0, 0, B)
#define MAKE_CYAN(C) make_rgb(0, C, C)
#define MAKE_MAGENTA(M) make_rgb(M, 0, M)
#define MAKE_YELLOW(Y) make_rgb(Y, Y, 0)
#define MAKE_GRAY(L) make_rgb(L, L, L)

namespace roads {

    rgb cell::palette[256] = {
        cell::death_color, cell::life_color, cell::slow_color, cell::fast_color, cell::ice_color,
        MAKE_GRADIENT(MAKE_GRAY),
        MAKE_GRADIENT(MAKE_RED),
        MAKE_GRADIENT(MAKE_BLUE),
        MAKE_GRADIENT(MAKE_GREEN),
        MAKE_GRADIENT(MAKE_CYAN),
        MAKE_GRADIENT(MAKE_MAGENTA),
        MAKE_GRADIENT(MAKE_YELLOW),
    };

    inline rgb scale_rgb(rgb color, f32 scale) {
        int16_t r = color & 31;
        int16_t g = (color >> 5) & 31;
        int16_t b = (color >> 10) & 31;
        r = int16_t((scale * r).to_int());
        g = int16_t((scale * g).to_int());
        b = int16_t((scale * b).to_int());
        return make_rgb(r, g, b);
    }

    disp_writer& operator<<(disp_writer& writer, draw_cell const& drc) {
        auto saved = writer.save();

        constexpr f16 block = geometry::draw::block_size;
        constexpr f16 tile = geometry::draw::tile_height;

        vector3f16 const scale = drc.scale;
        f16 const back = scale.z * -block;

        cell const c = drc.c;
        vector3f16 const offset { drc.position.x, c.altitude * geometry::draw::block_size * f16(0.5), 0 };
        vector3f16 const back_offset = offset + vector3f16{0, 0, back};
        rgb const ambient = make_rgb(0, 0, 0);
        rgb const tilec = scale_rgb(tile_color(c), f16(0.5));
        rgb const blockc = scale_rgb(block_color(c), f16(0.5));

        writer << specular_emission { make_rgb(0, 0, 0), make_rgb(0, 0, 0), false };

        if(c.flags & cell::tile) {
            // tile color
            writer
                << diffuse_ambient { tilec, ambient, false };

            if(!(c.flags & cell::low || c.flags & cell::high)) {
                    // tile top
                writer
                    << normal { { 0, 1, 0 } }
                    << quad { offset + vector3f16{ 0,     tile, 0 },
                              offset + vector3f16{ block, tile, 0 },
                              offset + vector3f16{ block, tile, back },
                              offset + vector3f16{ 0,     tile, back } };
            }

            writer
                // tile front
                << normal { { 0, 0, 1 } }
                << quad { offset + vector3f16{ 0,     tile, 0 },
                          offset + vector3f16{ 0,     0,    0 },
                          offset + vector3f16{ block, 0,    0 },
                          offset + vector3f16{ block, tile, 0 } }
                // tile left side
                << normal { { -1, 0, 0 } }
                << quad { offset + vector3f16{ 0,     0, back },
                          offset + vector3f16{ 0,     0, 0 },
                          offset + vector3f16{ 0,     tile,  0 },
                          offset + vector3f16{ 0,     tile,  back } }
                // tile right side
                << normal { { 1, 0, 0 } }
                << quad { offset + vector3f16{ block, tile, back },
                          offset + vector3f16{ block, tile, 0 },
                          offset + vector3f16{ block, 0,  0 },
                          offset + vector3f16{ block, 0,  back } };
        }
        if(c.flags & cell::tunnel) {
            using geometry::tunnel::inner;

            vector3f16 const (&outer)[7] =
                (c.flags & cell::low)
                ? geometry::tunnel::low_outer
                : (c.flags & cell::high)
                ? geometry::tunnel::high_outer
                : geometry::tunnel::outer;

            // front
            writer
                << diffuse_ambient { blockc, ambient, false }
                << normal { { 0, 0, 1 } }
                << quad_strip {
                    outer[0] + offset, inner[0] + offset,
                    outer[1] + offset, inner[1] + offset,
                    outer[2] + offset, inner[2] + offset,
                    outer[3] + offset, inner[3] + offset,
                    outer[4] + offset, inner[4] + offset,
                    outer[5] + offset, inner[5] + offset,
                    outer[6] + offset, inner[6] + offset,
                };

            // top

            if((c.flags & cell::high) || (c.flags & cell::high)) {
                writer
                    << normal { { 1, 0, 0 } }
                    << quad { outer[0] + back_offset, outer[0] + offset, outer[2] + offset, outer[2] + back_offset }
                    << normal { { 0, 1, 0 } }
                    << quad { outer[2] + back_offset, outer[2] + offset, outer[4] + offset, outer[4] + back_offset }
                    << normal { {-1, 0, 0 } }
                    << quad { outer[4] + back_offset, outer[4] + offset, outer[6] + offset, outer[6] + back_offset };
            }
            else {
                using geometry::tunnel::normals;

                writer <<
                    arc({
                          { normals[0], outer[0] + back_offset, outer[0] + offset },
                          { normals[1], outer[1] + back_offset, outer[1] + offset },
                          { normals[2], outer[2] + back_offset, outer[2] + offset },
                          { normals[3], outer[3] + back_offset, outer[3] + offset },
                          { normals[4], outer[4] + back_offset, outer[4] + offset },
                          { normals[5], outer[5] + back_offset, outer[5] + offset },
                          { normals[6], outer[6] + back_offset, outer[6] + offset },
                        });
            }
        }
        else if((c.flags & cell::low) || (c.flags & cell::high)) {
            bool const has_tile = c.flags & cell::tile;
            f16 const top = (c.flags & cell::low)
                ? geometry::draw::short_height + tile
                : geometry::draw::tall_height + tile;
            f16 const bottom = has_tile ? tile : 0;

            writer
                << diffuse_ambient { blockc, ambient, false }
                // block top
                << normal { { 0, 1, 0 } }
                << quad { offset + vector3f16{ 0,     top, 0 },
                          offset + vector3f16{ block, top, 0 },
                          offset + vector3f16{ block, top, back },
                          offset + vector3f16{ 0,     top, back } }
                // block front
                << normal { { 0, 0, 1 } }
                << quad { offset + vector3f16{ 0,     top,  0 },
                          offset + vector3f16{ 0,     bottom, 0 },
                          offset + vector3f16{ block, bottom, 0 },
                          offset + vector3f16{ block, top,  0 } }
                // block left side
                << normal { { -1, 0, 0 } }
                << quad { offset + vector3f16{ 0,     bottom, back },
                          offset + vector3f16{ 0,     bottom, 0 },
                          offset + vector3f16{ 0,     top,  0 },
                          offset + vector3f16{ 0,     top,  back } }
                // block right side
                << normal { { 1, 0, 0 } }
                << quad { offset + vector3f16{ block, top, back },
                          offset + vector3f16{ block, top, 0 },
                          offset + vector3f16{ block, bottom,  0 },
                          offset + vector3f16{ block, bottom,  back } };

        }

        if(!writer)
            writer.reset(saved);
        return writer;
    }

}
