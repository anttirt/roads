#include <boost/lexical_cast.hpp>

#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "disp_writer.h"
#include "utility.h"
#include <nds.h>

constexpr uint32_t make_diffuse_ambient(uint8_t dr, uint8_t dg, uint8_t db, uint8_t ar, uint8_t ag, uint8_t ab, bool set_vertex_color) {
    return RGB15(dr, dg, db) | (RGB15(ar, ag, ab) << 16) | ((set_vertex_color ? 1 : 0) << 15);
}

#define FIFO_MATRIX_CONTROL   REG2ID(MATRIX_CONTROL)
#define FIFO_MATRIX_PUSH      REG2ID(MATRIX_PUSH)
#define FIFO_MATRIX_POP       REG2ID(MATRIX_POP)
#define FIFO_MATRIX_SCALE     REG2ID(MATRIX_SCALE)
#define FIFO_MATRIX_TRANSLATE REG2ID(MATRIX_TRANSLATE)
#define FIFO_MATRIX_RESTORE   REG2ID(MATRIX_RESTORE)
#define FIFO_MATRIX_STORE     REG2ID(MATRIX_STORE)
#define FIFO_MATRIX_IDENTITY  REG2ID(MATRIX_IDENTITY)
#define FIFO_MATRIX_LOAD4x4   REG2ID(MATRIX_LOAD4x4)
#define FIFO_MATRIX_LOAD4x3   REG2ID(MATRIX_LOAD4x3)
#define FIFO_MATRIX_MULT4x4   REG2ID(MATRIX_MULT4x4)
#define FIFO_MATRIX_MULT4x3   REG2ID(MATRIX_MULT4x3)
#define FIFO_MATRIX_MULT3x3   REG2ID(MATRIX_MULT3x3)

namespace {
uint32_t const disp_lst[] = {
    FIFO_COMMAND_PACK(FIFO_MATRIX_PUSH, FIFO_MATRIX_IDENTITY, FIFO_MATRIX_LOAD4x3, FIFO_NOP),
    1 << 12, 0,       0,              //
    0,       1 << 12, 0,              // 4x3 transformation matrix:
    0,       0,       1 << 12,        // scale (1:1) and translate (0,0,-5)
    0,       0,       (1 << 12) * -5, //
    0, // nop
    FIFO_COMMAND_PACK(FIFO_DIFFUSE_AMBIENT, FIFO_SPECULAR_EMISSION, FIFO_NORMAL, FIFO_BEGIN),
    make_diffuse_ambient(24, 24, 24, 3, 3, 3, true),
    make_diffuse_ambient(0, 0, 0, 0, 0, 0, false),
    NORMAL_PACK(floattov10(0.0), floattov10(0.0), floattov10(-1.) & 0x3FF),
    GL_QUAD,
    // the 0x3FF mask here is just to satisfy the test; the hardware doesn't actually care about the top two bits

    FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_VERTEX16, FIFO_VERTEX16, FIFO_VERTEX16),
    VERTEX_PACK(inttov16(-1),inttov16(1)), VERTEX_PACK(0,0),
    VERTEX_PACK(inttov16(-1),inttov16(-1)), VERTEX_PACK(0,0),
    VERTEX_PACK(inttov16(1),inttov16(-1)), VERTEX_PACK(0,0),
    VERTEX_PACK(inttov16(1),inttov16(1)), VERTEX_PACK(0,0),

    FIFO_COMMAND_PACK(FIFO_MATRIX_POP, FIFO_NOP, FIFO_NOP, FIFO_NOP),
    1, 0, 0, 0
};

}

namespace roads {
    namespace {
        UNIT_TEST(write_quad,
        {
            uint32_t buf[1024];
            disp_writer writer(buf, buf + 1024, { 0, 0, -5 }, { 1, 1, 1 });
            UASSERT_EQUAL(writer.write_count(), 14);
            UASSERT_EQUAL(writer.get_pipe_index(), 0);
            UASSERT(bool(writer), "Writer not OK after writing prelude");
            writer
                << diffuse_ambient { make_rgb(24, 24, 24), make_rgb(3, 3, 3), true }
                << specular_emission { make_rgb(0, 0, 0), make_rgb(0, 0, 0), false }
                << normal { { 0, 0, -1 } };
            UASSERT_EQUAL(writer.get_pipe_index(), 3);
            UASSERT(bool(writer), "Writer not OK after pushing three commands");
            UASSERT_EQUAL(writer.write_count(), 14);
            writer
                << quad { { -1, 1, 0 }, { -1, -1, 0 }, { 1, -1, 0 }, { 1, 1, 0 } };
            UASSERT(bool(writer), "Writer not OK after writing quad");
            writer
                << end;

            UASSERT_EQUAL(writer.write_count(), countof(disp_lst));

            for(size_t i = 0; i < countof(disp_lst); ++i) {
                UASSERT(buf[i] == disp_lst[i], "[%d] %X != %X", i, buf[i], disp_lst[i]);
            }
        });

        template <typename T>
        std::auto_ptr<unit_test_base> make_auto(T* p) { return std::auto_ptr<unit_test_base>(p); }
    }

    template <>
    void create_tests<disp_writer>(unit_test_suite& suite)
    {
        suite.add_test(make_auto(new write_quad));
    }
}

#endif

