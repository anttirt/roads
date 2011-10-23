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
        void draw() const;
        void update(f32 position);

        enum {
            draw_distance = 25
        };

    private:
        grid_t grid;
        grid_t::iterator visible_start, visible_end;
        draw_queue_t draw_queue;
        // pool up our display data buffers to avoid unnecessary allocations
        draw_queue_t draw_pool;


        display_row get_display_row();

        display_row generate_row_display_list(grid_t::iterator rowp);
    };
}


#endif // ROADS_LEVEL_H

