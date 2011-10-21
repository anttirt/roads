#ifndef DSR_CELL_H
#define DSR_CELL_H

#include <stdint.h>
#include <cstddef>

//#include "aabb.h"

namespace roads
{
    typedef unsigned short rgb;

	template <uint8_t R, uint8_t G, uint8_t B>
	struct make_rgb
	{
		static rgb const value = rgb(R | (G << 5) | (B << 10));
	};
    /*
    cell structure

    - needs?
        - simple, POD, clearly defined layout
        - easy to (de)serialize
        - describes two colors, cell type, altitude:
            - has a bottom tile? short block? tall block? tunnel?
            - tile color separate from tunnel/block color:
                - gotta have possibility for safe inside but not safe outside
            - colors also describe behavior (core mechanic from original game)
                - red = death, blue = life, green = fast, dark green = slow, gray = ice
                - other colors not meaningful; just to look nice
                - color format 16-bit rgb; would make for at least 48-bit cell structure
                    - palette indexing with two 8-bit values
        - cell structure is 32 bits in size
            - add a union to uint32_t for quick and easy comparisons

    - methods
        - enums for cell type and special colors
            - cell types are bit flags
                - add suitable operators to avoid casting in client code
        - all colors in a LUT; significant (with behavior) ones found at the start
        - main members use union { struct { members; }; total_value; }; approach
        - ensure size and no funny alignment/packing business with a template check
            - fails to compile if everything in the cell doesn't fit in a uint32_t
    */

    // Describes a single cell in the game's 7 x N grid.
    // Precisely 32 bits long with two eight-bit palette indices,
    // an eight-bit altitude number and one byte of bit flags
    // to determine the type of the cell.
    struct cell
    {
        // {{{ For two cells c1 and c2 where c2 is further
        // from the camera, draw the face of c2 facing the
        // camera only if:
        // c1.altitude != c2.altitude || (c2.flags & elide_flags) > (c1.flags & elide_flags)
        // ie. the further away one
        // a) doesn't have equal altitude to
        // b) has features not hidden by
        // the closer one. }}}
        /* {{{ */enum cellflags_t
        {
            none = 0x00,
            tile = 0x01,
            low = 0x02,
            tunnel = 0x04,
            high = 0x08,
            end = 0x10,

            // This flag is a special one that's used
            // during runtime to mark the cells that
            // have already been processed for drawing
            // and added to a display list.
            runtime_drawn = 0x20,

            elide_flags = 0x01 | 0x02 | 0x04 | 0x08
        }; /* }}} */
/*
        static inline v16 fv16(float f)
        {
            return v16(f * (1 << 12));
        }
*/
        /*
        unsigned bounds(aabb* bounds) const
        {
            f16 const ratio = 1.f / 6.f;
            f16 const n_ratio = 5.f / 6.f;
            f16 const unit = 1.f;

            if(flags & tunnel)
            {
                bool had_tile = false;

                if(flags & tile)
                {
                    bounds->min = vec(0, 0, 0);
                    bounds->max = vec(unit, ratio,unit);
                    had_tile = true;
                    ++bounds;
                }

                bounds->min = vec(0, ratio, 0);
                bounds->max = vec(ratio, n_ratio, unit);
                ++bounds;

                bounds->min = vec(n_ratio, ratio, 0);
                bounds->max = vec(unit, n_ratio, unit);
                ++bounds;

                bounds->min = vec(0, n_ratio, 0);
                if(flags & high)
                    bounds->max = vec(unit, unit * 2, unit);
                else
                    bounds->max = vec(unit, unit, unit);

                return 3 + (had_tile ? 1 : 0);
            }

            switch(flags & ~(end | runtime_drawn))
            {
                case tile:
                {
                    bounds[0].min = vec(0, 0, 0);
                    bounds[0].max = vec(unit, ratio, unit);

                    return 1;
                }
                case low:
                {
                    bounds[0].min = vec(0, ratio, 0);
                    bounds[0].max = vec(unit, unit, unit);

                    return 1;
                }

                case high:
                {
                    bounds[0].min = vec(0, ratio, 0);
                    bounds[0].max = vec(unit, unit * 2, unit);

                    return 1;
                }

                case tile | low:
                {
                    bounds[0].min = vec(0, 0, 0);
                    bounds[0].max = vec(unit, unit, unit);

                    return 1;
                }

                case tile | high:
                {
                    bounds[0].min = vec(0, 0, 0);
                    bounds[0].max = vec(unit, unit * 2, unit);

                    return 1;
                }

                default:
                {
                    return 0;
                }
            }
        }
        */

        /* {{{ */ enum // color values
        {
			death_color = make_rgb<31, 0, 0>::value,
            life_color = make_rgb<0, 0, 31>::value,
            slow_color = make_rgb<0, 15, 0>::value,
            fast_color = make_rgb<0, 31, 0>::value,
            ice_color = make_rgb<6, 6, 6>::value
        }; /* }}} */
        /* {{{ */ enum // color indices
        {
            death = 0,
            life = 1,
            slow = 2,
            fast = 3,
            ice = 4,
        }; /* }}} */
        /* {{{ */ union // cell data
        {
            struct
            {
                uint8_t tile_color, block_color;
                uint8_t altitude;
                cellflags_t flags;
            };
            uint32_t cell_value;
        }; /* }}} */
        
        // Draw faces facing to the camera only when required:
        // if(far.face_required(near))
        //     draw_z(far);
        /* {{{ */ bool face_required(cell near) const
        {
            return
                (near.altitude != altitude) ||
                ((flags & elide_flags) > (near.flags & elide_flags));
        } /* }}} */

        static rgb palette[256];
    };

    // {{{ cell relational ops
    inline bool operator<(cell lhs, cell rhs) { return lhs.cell_value < rhs.cell_value; }
    inline bool operator>(cell lhs, cell rhs) { return lhs.cell_value > rhs.cell_value; }
    inline bool operator<=(cell lhs, cell rhs) { return lhs.cell_value <= rhs.cell_value; }
    inline bool operator>=(cell lhs, cell rhs) { return lhs.cell_value >= rhs.cell_value; }
    inline bool operator==(cell lhs, cell rhs) { return lhs.cell_value == rhs.cell_value; }
    inline bool operator!=(cell lhs, cell rhs) { return lhs.cell_value != rhs.cell_value; }
    // }}}
    
    inline rgb block_color(cell c) { return cell::palette[c.block_color]; }
    inline rgb tile_color(cell c) { return cell::palette[c.tile_color]; }

    namespace detail
    {
        template <bool Success> struct ensure_cell_size;
        template <> struct ensure_cell_size<true> { typedef int type; };
        typedef ensure_cell_size<sizeof(cell) == sizeof(uint32_t)>::type ensure_cell_size_type;
    }
    // {{{ cell flags bitwise ops
    typedef cell::cellflags_t cellflags_t;

    inline cellflags_t operator|(cellflags_t lhs, cellflags_t rhs) { return cellflags_t(unsigned(lhs) | unsigned(rhs)); }
    inline cellflags_t operator&(cellflags_t lhs, cellflags_t rhs) { return cellflags_t(unsigned(lhs) & unsigned(rhs)); }
    inline cellflags_t operator^(cellflags_t lhs, cellflags_t rhs) { return cellflags_t(unsigned(lhs) ^ unsigned(rhs)); }
    inline cellflags_t& operator|=(cellflags_t& lhs, cellflags_t rhs) { return lhs = lhs | rhs; }
    inline cellflags_t& operator&=(cellflags_t& lhs, cellflags_t rhs) { return lhs = lhs & rhs; }
    inline cellflags_t& operator^=(cellflags_t& lhs, cellflags_t rhs) { return lhs = lhs ^ rhs; }
    // }}}


}

#endif // DSR_CELL_H
