#ifndef DSR_GLCORE_H
#define DSR_GLCORE_H

#include "utility.h"
#include "address_transform.h"

namespace roads
{
    enum { gfx_address_base = 0x04000000 };

    enum gfx_offset_t
    {
        gfx_control           = 0x060,

        gfx_fifo              = 0x400,
        gfx_nop               = 0x400,
        gfx_status            = 0x600,

        gfx_matrix_mode       = 0x440,
        gfx_matrix_push       = 0x444,
        gfx_matrix_pop        = 0x448,
        gfx_matrix_store      = 0x44c,
        gfx_matrix_restore    = 0x450,
        gfx_matrix_identity   = 0x454,
        gfx_matrix_load_4x4   = 0x458,
        gfx_matrix_load_4x3   = 0x45c,
        gfx_matrix_mult_4x4   = 0x460,
        gfx_matrix_mult_4x3   = 0x464,
        gfx_matrix_mult_3x3   = 0x468,
        gfx_matrix_scale      = 0x46c,
        gfx_matrix_trans      = 0x470,

        gfx_color             = 0x480,
        gfx_vertex10          = 0x490,
        gfx_vertex_xy         = 0x494,
        gfx_vertex_xz         = 0x498,
        gfx_vertex_yz         = 0x49c,
        gfx_vertex_diff       = 0x4a0,

        gfx_vertex16          = 0x48c,
        gfx_tex_coord         = 0x488,
        gfx_tex_format        = 0x4a8,
        gfx_pal_format        = 0x4ac,

        gfx_clear_color       = 0x350,
        gfx_clear_depth       = 0x354,

        gfx_light_vector      = 0x4c8,
        gfx_light_color       = 0x4cc,
        gfx_normal            = 0x484,

        gfx_diffuse_ambient   = 0x4c0,
        gfx_specular_emission = 0x4c4,
        gfx_shininess         = 0x4d0,

        gfx_poly_format       = 0x4a4,
        gfx_alpha_test        = 0x340,

        gfx_begin             = 0x500,
        gfx_end               = 0x504,
        gfx_flush             = 0x540,
        gfx_viewport          = 0x580,
        gfx_toon_table        = 0x380,
        gfx_edge_table        = 0x330,
        gfx_fog_color         = 0x358,
        gfx_fog_offset        = 0x35c,
        gfx_fog_table         = 0x360,
        gfx_box_test          = 0x5c0,
        gfx_pos_test          = 0x5c4,
        gfx_pos_result        = 0x620,

        gfx_vertex_ram_usage  = 0x606,
        gfx_polygon_ram_usage = 0x604,
        gfx_cutoff_depth      = 0x610
    };

    enum gl_begin_t
    {
        gl_triangles      = 0,
        gl_quads          = 1,
        gl_triangle_strip = 2,
        gl_quad_strip     = 3,
    };

    namespace detail
    {
        // Base template, this is what most registers have.
        template <gfx_offset_t Offset> struct reg_type       : apply_op< uint32_t       volatile &      > { };

        //                           register                            base     cv-qualifiers  ref/ptr/array
        template <> struct reg_type< gfx_control           > : apply_op< uint16_t       volatile &      > { };
        template <> struct reg_type< gfx_clear_depth       > : apply_op< uint16_t       volatile &      > { };
        template <> struct reg_type< gfx_alpha_test        > : apply_op< uint16_t       volatile &      > { };
        template <> struct reg_type< gfx_toon_table        > : apply_op< uint16_t       volatile [0x20] > { }; // size = 0x40 bytes
        template <> struct reg_type< gfx_edge_table        > : apply_op< uint16_t       volatile [0x08] > { }; // size = 0x10 bytes
        template <> struct reg_type< gfx_fog_table         > : apply_op< uint8_t        volatile [0x20] > { }; // size = 0x20 bytes
        template <> struct reg_type< gfx_box_test          > : apply_op< int32_t        volatile &      > { };
        template <> struct reg_type< gfx_pos_result        > : apply_op< int32_t  const volatile [0x04] > { }; // size = 0x10 bytes
        template <> struct reg_type< gfx_vertex_ram_usage  > : apply_op< uint16_t                       > { };
        template <> struct reg_type< gfx_polygon_ram_usage > : apply_op< uint16_t                       > { };
        template <> struct reg_type< gfx_cutoff_depth      > : apply_op< uint16_t                &      > { };
    }

    template <gfx_offset_t Offset>
    // Choose result type from above specializations
    typename detail::reg_type<Offset>::result_type
    gfx_reg()
    {
        typedef typename detail::reg_type<Offset>::addr_type addr_type;
        typedef typename detail::reg_type<Offset>::result_type result_type;

        // All enumeration values are specified as offsets from gfx_address_base (0x04000000, also the general hardware IO address base)
        uint32_t const addr = gfx_address_base + Offset;
        addr_type ptr = reinterpret_cast<addr_type>(addr);
        return detail::reg_type<Offset>::apply(ptr);
    }

    inline constexpr uint8_t id(gfx_offset_t offset)
    {
        return static_cast<uint8_t>((offset - 0x400) >> 2);
    }

    inline constexpr uint32_t fifo_pack(gfx_offset_t c1, gfx_offset_t c2, gfx_offset_t c3, gfx_offset_t c4)
    {
        return
            (id(c4) << 24) | 
            (id(c3) << 16) | 
            (id(c2) << 8) | 
            (id(c1));
    }

    inline constexpr uint32_t pack_material(rgb dif, rgb amb, bool flag) {
        return dif | (amb << 16) | ((flag ? 0 : 1) << 15);
    }

}

#endif // DSR_GLCORE_H

