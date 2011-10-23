#ifndef ROADS_DISP_WRITER_H
#define ROADS_DISP_WRITER_H

#include <cstdint>
#include <cassert>
#include "glcore.h"
#include "fixed16.h"
#include "vector.h"

namespace roads {
    /*
     * Usage:
     *
     * The following example writes a medium-to-light grey quad on the xy plane
     * with normals pointing in the negative z direction, with the quad itself
     * translated in the negative z direction by 5 units.
     *
     *     uint32_t buf[1024];
     *     disp_writer writer(buf, buf + 1024, { 0, 0, -5 }, { 1, 1, 1 });
     *     writer
     *         << diffuse_ambient { make_rgb(0.6, 0.6, 0.6), make_rgb(0, 0, 0) }
     *         << normal { { 0, 0, -1 } }
     *         << quad { { -1, 1, 0 }, { -1, -1, 0 }, { 1, -1, 0 }, { 1, 1, 0 } }
     *         << end;
     *
     * If you're doing a sequence of several operations and want to make it
     * either all succeed or all fail, you can use the save and reset member
     * functions:
     *
     *     disp_writer& write_four(thing a, thing b, thing c, thing d) {
     *         auto state = writer.save();
     *         writer << a << b << c << d;
     *         if(!writer) {
     *             writer.reset(state);
     *         }
     *         return writer;
     *     }
     *
     * Any commands that were written out to the buffer after the state save
     * will not be cleared out, but the write pointer of the disp_writer will
     * be reset so that it overwrites them on further writes.
     *
     */

    struct disp_writer {
        typedef uint32_t* iterator;

        enum {
            min_buffer_length = 1 /* cmdpack */ + 12 /* 4x3 matrix */ + 2 /* nops to fill first param pack */, 
            matrix_start_offset = 1
        };

        // The display list format packs four commands into one uint32_t, after
        // which come all the parameters for those commands. In order to
        // properly pack the display list into the buffer, we keep a staging
        // area ("pipe") of four commands here which then get written out in
        // one go when the function flush is called, either manually or
        // automatically when the pipe becomes full.
        struct cmd {
            uint32_t params[2];
            gfx_offset_t offset;
            uint8_t pcount;
        } pipe[4];

        struct reset_data {
            cmd pipe[4];
            iterator buffer_pos;
            size_t pipe_index;
        };

        reset_data save() {
            return reset_data { { pipe[0], pipe[1], pipe[2], pipe[3] }, buffer_pos, pipe_index };
        }

        void reset(reset_data const& data) {
            pipe[0] = data.pipe[0];
            pipe[1] = data.pipe[1];
            pipe[2] = data.pipe[2];
            pipe[3] = data.pipe[3];
            buffer_pos = data.buffer_pos;
            pipe_index = data.pipe_index;
        }

        // Returns false if the buffer has become full. You can test a writer's
        // state by converting it to bool or using it in a boolean context,
        // such as an if-statement, much like standard streams:
        //
        //     if(!(writer << x)) {
        //         process failure;
        //     }
        explicit operator bool() {
            return !buffer_full;
        }

        size_t get_pipe_index() {
            return pipe_index;
        }

        size_t write_count() {
            return buffer_pos - buffer_start;
        }

    private:
        iterator buffer_start, buffer_pos, buffer_end;
        size_t pipe_index;

        bool buffer_full;

        void append(uint32_t value) {
            *buffer_pos++ = value;
        }

    public:
        // Flushes the pipe into the buffer. If the buffer doesn't have enough
        // space, we return false and set buffer_full to true, which allows the
        // user to reallocate more space and reset the buffer pointers, after
        // which the user can try flushing the pipe again.
        //
        // (2) We never write fewer than four commands but according to the
        // spec at http://nocash.emubase.de/gbatek.htm the "top-most packed
        // command MUST have parameters" so if we get a command with zero
        // parameters in that slot, we will put a nop in that command's place
        // and defer it to the next flush.
        disp_writer& flush_pipe() {
            // Insert nops if we don't have a full pack of four commands.
            for(size_t i = pipe_index; i < 4; ++i) {
                pipe[i].offset = gfx_nop;
                pipe[i].pcount = 1;
                pipe[i].params[0] = 0;
            }

            pipe_index = 4;

            bool defer = false;
            cmd deferred {};

            // See paragraph (2) above.
            if(pipe[3].pcount == 0) {
                defer = true;
                deferred = pipe[3];
                pipe[3].offset = gfx_nop;
                pipe[3].pcount = 1;
                pipe[3].params[0] = 0;
            }

            // Check if we have enough space left in the buffer. If we don't,
            // set the buffer_full flag, restore any deferred command and
            // return early.
            ptrdiff_t size_needed = 1 /* pack */ + pipe[0].pcount + pipe[1].pcount + pipe[2].pcount + pipe[3].pcount;
            if(buffer_end - buffer_pos < size_needed) {
                if(defer) {
                    pipe[3] = deferred;
                }
                buffer_full = true;
                return *this;
            }

            uint32_t pack = fifo_pack(pipe[0].offset, pipe[1].offset, pipe[2].offset, pipe[3].offset);
            append(pack);
            for(size_t i = 0; i < 4; ++i) {
                for(size_t j = 0; j < pipe[i].pcount; ++j)
                    append(pipe[i].params[j]);
            }

            // See paragraph (2) above. 0 is an invalid command.
            if(defer) {
                pipe[0] = deferred;
                pipe_index = 1;
            }
            else {
                pipe_index = 0;
            }

            return *this;
        }

        // The user can call this function with a new buffer to continue
        // writing after a buffer has been filled.
        disp_writer& reseat_buffer(iterator new_buffer_start, iterator new_buffer_end) {
            buffer_start = buffer_pos = new_buffer_start;
            buffer_end = new_buffer_end;
            buffer_full = false;
            return *this;
        }

        disp_writer& push(gfx_offset_t cmd) {
            if(pipe_index == 4 && !flush_pipe())
                return *this;
            pipe[pipe_index].offset = cmd;
            pipe[pipe_index].pcount = 0;
            ++pipe_index;
            return *this;
        }

        disp_writer& push(gfx_offset_t cmd, uint32_t param0) {
            if(pipe_index == 4 && !flush_pipe())
                return *this;
            pipe[pipe_index].offset = cmd;
            pipe[pipe_index].pcount = 1;
            pipe[pipe_index].params[0] = param0;
            ++pipe_index;
            return *this;
        }

        disp_writer& push(gfx_offset_t cmd, uint32_t param0, uint32_t param1) {
            if(pipe_index == 4 && !flush_pipe())
                return *this;
            pipe[pipe_index].offset = cmd;
            pipe[pipe_index].pcount = 2;
            pipe[pipe_index].params[0] = param0;
            pipe[pipe_index].params[1] = param1;
            ++pipe_index;
            return *this;
        }

    private:
        void push_prelude(vector3f32 const& pos, vector3f32 const& scale) {
            append(fifo_pack(gfx_matrix_push, gfx_matrix_mult_4x3, gfx_nop, gfx_nop));
            // no params for push or identity

            // params for load_4x3
            append(raw(scale.x)); append(           0); append(           0);
            append(           0); append(raw(scale.y)); append(           0);
            append(           0); append(           0); append(raw(scale.z));
            append(  raw(pos.x)); append(  raw(pos.y)); append(  raw(pos.z));

            // param for nop
            append(0);
            append(0);
        }

    public:
        disp_writer(iterator buffer_start, iterator buffer_end, vector3f32 const& translation, vector3f32 const& scale)
            : pipe(), buffer_start(buffer_start), buffer_pos(buffer_start), buffer_end(buffer_end),
              pipe_index(0), buffer_full(false)
        {
            assert(buffer_end - buffer_start >= min_buffer_length);
            push_prelude(translation, scale);
        }

        disp_writer& finish() {
            push(gfx_matrix_pop, 1);
            flush_pipe();
            return *this;
        }
    };

    struct writer_end_t {};
    constexpr writer_end_t end {};
    struct writer_nop_t {};
    constexpr writer_nop_t nop {};

    struct quad {
        vector3f16 a, b, c, d;
    };
    struct nquad {
        vertex a, b, c, d;
    };
    struct normal {
        constexpr normal(vector3f16 const& data) : data(data) {}
        vector3f16 data;
    };
    struct diffuse_ambient {
        int16_t diffuse, ambient;
        bool set_vertex_color;
    };
    struct specular_emission {
        int16_t specular, emission;
        bool enable_shininess_table;
    };

    inline disp_writer& operator<<(disp_writer& writer, vector3f16 vertex) {
        return writer.push(gfx_vertex16, vertex_pack(vertex.x, vertex.y), vertex_pack(vertex.z, 0));
    }

    inline disp_writer& operator<<(disp_writer& writer, vertex const& v) {
        auto saved = writer.save();
        writer.push(gfx_normal, normal_pack(v.normal)) << v.position;
        if(!writer)
            writer.reset(saved);
        return writer;
    }

    inline disp_writer& raw_vertex(disp_writer& writer, vertex const& v) {
        // caller takes care of state
        return writer.push(gfx_normal, normal_pack(v.normal)) << v.position;
    }

    inline disp_writer& operator<<(disp_writer& writer, normal const& n) {
        return writer.push(gfx_normal, normal_pack(n.data));
    }

    inline disp_writer& operator<<(disp_writer& writer, diffuse_ambient diff_amb) {
        return writer.push(gfx_diffuse_ambient,
                             (uint32_t(diff_amb.diffuse) & 0xFFFF)
                           | (uint32_t(diff_amb.ambient) << 16)
                           | ((diff_amb.set_vertex_color ? 1 : 0) << 15) );
    }
    inline disp_writer& operator<<(disp_writer& writer, specular_emission spec_emis) {
        return writer.push(gfx_specular_emission,
                             (uint32_t(spec_emis.specular) & 0xFFFF)
                           | (uint32_t(spec_emis.emission) << 16)
                           | ((spec_emis.enable_shininess_table ? 1 : 0) << 15) );
    }

    inline disp_writer& operator<<(disp_writer& writer, nquad const& q) {
        auto saved = writer.save();
        writer.push(gfx_begin, gl_quads);
        raw_vertex(writer, q.a);
        raw_vertex(writer, q.b);
        raw_vertex(writer, q.c);
        raw_vertex(writer, q.d);
        if(!writer)
            writer.reset(saved);
        return writer;
    }

    inline disp_writer& operator<<(disp_writer& writer, writer_nop_t) {
        return writer.push(gfx_nop, 0);
    }

    inline disp_writer& operator<<(disp_writer& writer, writer_end_t) {
        return writer.finish();
    }

    inline disp_writer& operator<<(disp_writer& writer, quad const& q) {
        auto saved = writer.save();
        writer.push(gfx_begin, gl_quads) << q.a << q.b << q.c << q.d;
        if(!writer)
            writer.reset(saved);
        return writer;
    }

}

#endif // ROADS_DISP_WRITER_H

