#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "display_list.h"
#include "utility.h"

#include <boost/lexical_cast.hpp>
#include <nds.h>

constexpr uint32_t diffuse_ambient(uint8_t dr, uint8_t dg, uint8_t db, uint8_t ar, uint8_t ag, uint8_t ab, bool set_vertex_color) {
    return RGB15(dr, dg, db) | (RGB15(ar, ag, ab) << 16) | ((set_vertex_color ? 0 : 1) << 15);
}

uint32_t const disp_lst[] = {
    FIFO_COMMAND_PACK(FIFO_DIFFUSE_AMBIENT, FIFO_SPECULAR_EMISSION, FIFO_BEGIN, FIFO_TEX_COORD),
    diffuse_ambient(24, 24, 24, 3, 3, 3, true),
    diffuse_ambient(0, 0, 0, 0, 0, 0, false),
    GL_TRIANGLE,
    TEXTURE_PACK(inttot16(0), inttot16(0)),

    FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX16, FIFO_TEX_COORD, FIFO_NORMAL),
    // the 0x3FF mask here is just to satisfy the test; the hardware doesn't actually care about the top two bits
    NORMAL_PACK(floattov10(0.0), floattov10(0.0), floattov10(-1.) & 0x3FF), 
    VERTEX_PACK(inttov16(-1),inttov16(-1)), VERTEX_PACK(0,0),
    TEXTURE_PACK(inttot16(128), inttot16(0)),
    NORMAL_PACK(floattov10(0.0), floattov10(0.0), floattov10(-1.) & 0x3FF),

    FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16),
    VERTEX_PACK(inttov16(1),inttov16(-1)), VERTEX_PACK(0,0),
    TEXTURE_PACK(inttot16(0), inttot16(128)),
    NORMAL_PACK(floattov10(0.0), floattov10(0.0), floattov10(-1.) & 0x3FF),
    VERTEX_PACK(inttov16(0),inttov16(1)), VERTEX_PACK(0,0),

    //FIFO_COMMAND_PACK(FIFO_NOP, FIFO_NOP, FIFO_NOP, FIFO_END),
    //0, 0, 0
};

namespace roads
{
    namespace
    {
        UNIT_TEST(display_generate,
        {
            disp_gen gen;
            gen.diffuse_ambient(RGB15(24, 24, 24), RGB15(3, 3, 3), true);
            gen.specular_emission(RGB15(0, 0, 0), RGB15(0, 0, 0), false);
            vertex v0 { vector3f16(-1, -1, 0), vector3f16(0, 0, -1), texcoord_t(inttot16(0), inttot16(0)) };
            vertex v1 { vector3f16( 1, -1, 0), vector3f16(0, 0, -1), texcoord_t(inttot16(128), inttot16(0)) };
            vertex v2 { vector3f16( 0,  1, 0), vector3f16(0, 0, -1), texcoord_t(inttot16(0), inttot16(128)) };
            gen.tri(v0, v1, v2);

            {
                display_list list;
                gen.append_to(list);

                UASSERT_EQUAL(list.size(), countof(disp_lst));

                size_t i = 0;
                for(auto cur = list.begin(), e = list.end(); cur != e; ++cur, ++i) {
                    UASSERT(disp_lst[i] == *cur, "diff found at\nindex %d:\n%X != %X", i, disp_lst[i], *cur);
                }
            }
        });

        template <typename T>
        std::auto_ptr<unit_test_base> make_auto(T* p) { return std::auto_ptr<unit_test_base>(p); }
    }

    template <>
    void create_tests<display_list>(unit_test_suite& suite)
    {
        suite.add_test(make_auto(new display_generate));
    }
}

#endif

