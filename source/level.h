#ifndef ROADS_LEVEL_H
#define ROADS_LEVEL_H

#include <array>
#include <vector>
#include <list>
#include <queue>
#include <algorithm>

#include "cell.h"
#include "vector.h"
#include "fixed16.h"
#include "display_list.h"
#include "disp_writer.h"
#include "geometry.h"

namespace roads {
    struct cell_aux {
        // As an optimization, during level loading each cell in the grid will
        // be augmented with the number of identical cells that follow it in
        // the upcoming rows, and the matching cells in the upcoming rows will
        // be marked with depth = 0 so that they will not be drawn at all.
        int depth;
        cell data;
    };
    typedef std::array<cell_aux, 7> row_t;
    typedef std::vector<row_t> grid_t;
    struct display_row {
        // Related to the above-mentioned optimization, each row will mark the
        // maximum depth of any of its cells so that the row's display data is
        // not prematurely purged when the player is moving forward.
        int depth;
        display_list data;

        display_row() : depth(), data() {}
        display_row(display_row&& rhs)
            : depth(rhs.depth), data(std::move(rhs.data)) { rhs.depth = 0; }
        display_row& operator=(display_row&& rhs) {
            depth = rhs.depth;
            data = std::move(rhs.data);
            rhs.depth = 0;
            return *this;
        }

        display_row(display_row const&) = delete;
        display_row& operator=(display_row const&) = delete;
    };
    typedef std::list<display_row> draw_queue_t;

    struct level {
        grid_t grid;
        grid_t::iterator visible_start, visible_end;
        draw_queue_t draw_queue;
        // pool up our display data buffers to avoid unnecessary allocations
        draw_queue_t draw_pool;

        enum {
            draw_distance = 25
        };

        void draw() const {
            for(display_row const& dl : draw_queue)
                if(dl.depth > 0)
                    dl.data.draw();
        }

        display_row get_display_row() {
            if(!draw_pool.empty()) {
                display_row ret = std::move(draw_pool.front());
                draw_pool.pop_front();
                return std::move(ret);
            }

            return display_row();
        }

        display_row generate_row_display_list(grid_t::iterator rowp) {
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
                f32(block_size) * std::distance(grid.begin(), rowp));

            disp_writer writer(result.data, translation, geometry::draw::scale);

            vector3f16 cell_offset { xoff, 0, 0 };
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

        void update(f32 position) {
            grid_t::iterator start = grid.begin() + std::min(floor(position / f32(geometry::draw::block_size)).to_int(), int32_t(grid.size()));
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
    };
}


#endif // ROADS_LEVEL_H
