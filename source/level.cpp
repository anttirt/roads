#include "level.h"

#include <algorithm>
#include "geometry.h"
#include "disp_writer.h"

namespace roads {

    void level::draw() const {
        for(display_row const& dl : draw_queue)
            if(dl.depth > 0)
                dl.data.draw();
    }

    display_row level::get_display_row() {
        if(!draw_pool.empty()) {
            display_row ret = std::move(draw_pool.front());
            draw_pool.pop_front();
            return std::move(ret);
        }

        return display_row();
    }

    display_row level::generate_row_display_list(grid_t::iterator rowp) {
        using geometry::draw::block_size;

        display_row result = get_display_row();
        result.depth = 0;
        // This is a rather liberal estimate and it should be possible to
        // cut it down quite a bit. With this the draw lists will take up a
        // maximum of 2048 * sizeof(uint32_t) * draw_distance = 200KiB of
        // memory.
        result.data.resize(2048); 
        // center x and set z distance to how far along the row is
        f16 const xoff = -(rowp->size() / 2.) * block_size;
        vector3f32 const translation(
            xoff,
            0,
            -f32(block_size) * std::distance(grid.begin(), rowp));

        disp_writer writer(result.data, translation, geometry::draw::scale);

        vector3f16 cell_offset { 0, 0, 0 };
        for(size_t i = 0; i < rowp->size(); ++i, cell_offset.x += block_size) {
            cell_aux aux = (*rowp)[i];
            if(aux.depth > 0) {
                result.depth = std::max(result.depth, aux.depth);
                writer << draw_cell { aux.data, cell_offset, { 1, 1, aux.depth } };
            }
        }

        writer << end;

        result.data.resize(writer.write_count());

        return std::move(result);
    }

    void level::update(f32 position) {
        // note: z is negative forward so we negate the position for grid indexing
        grid_t::iterator start = grid.begin() + std::min(floor(-position / f32(geometry::draw::block_size)).to_int(), int32_t(grid.size()));
        grid_t::iterator end = grid.begin() + std::min(std::distance(grid.begin(), start) + draw_distance, int32_t(grid.size()));

        if(start > visible_start) {
            // drop as many rows as we can from the start
            auto front = draw_queue.begin();
            for(; visible_start < start; ++visible_start) {
                if(front->depth == 0) {
                    draw_pool.push_back(std::move(draw_queue.front()));
                    draw_queue.pop_front();
                }
                else if(front->depth <= (start - visible_start)) {
                    draw_pool.push_back(std::move(draw_queue.front()));
                    draw_queue.pop_front();
                }
                else {
                    break;
                }
            }
            start = visible_start;
        }
        else if(start < visible_start) {
            // going backwards?
            // TODO
        }

        if(end > visible_end) {
            for(; visible_end < end; ++visible_end) {
                draw_queue.push_back(generate_row_display_list(visible_end));
            }
        }
        else if(end < visible_end) {
            // going backwards?
            // TODO
        }
    }
}

