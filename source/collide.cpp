#include "collide.h"

#include <algorithm>
#include "arrayvec.hpp"
#include "geometry.h"
#include "variant_access.hpp"


namespace roads {
    struct compare_sweep_result {
        bool operator()(roads::sweep_result_t const& l, roads::sweep_result_t const& r) {
            using namespace roads;
            using namespace sweep;
            return visit_binary<bool>(l, r,
                [](already a, already b) { return a.index < b.index; },
                [](already,   never)     { return true; },
                [](already,   from)      { return true; },

                [](from,      already)   { return false; },
                [](from a,    from b) -> bool {
                    if(a.index < b.index) {
                        return true;
                    }
                    else if(a.index > b.index) {
                        return false;
                    }
                    else {
                        return a.time < b.time;
                    }
                },
                [](from,      never)     { return true; },

                [](never,     already)   { return false; },
                [](never,     from)      { return false; },
                [](never,     never)     { return false; }
                );
        }
    };
    void select_tiles(vector3f32 const& position, arrayvec<vector2i, 8>& out);
    void make_bounds(vector2i position, grid_t const& grid, arrayvec<aabb, 32>& ret);
    void select_tiles(vector3f32 const& position, arrayvec<vector2i, 8>& out) {
        f32 const xoff = geometry::draw::ship_size.x;
        f32 const zoff = geometry::draw::ship_size.z;
        f32 const block_size = f32(geometry::draw::block_size);

        int x1 = floor(f32(3.5) + (position.x       ) / block_size).to_int();
        int x2 = floor(f32(3.5) + (position.x + xoff) / block_size).to_int();
        int z1 = floor(          -(position.z       ) / block_size).to_int();
        int z2 = floor(          -(position.z + zoff) / block_size).to_int();

        //z1 = -z1;
        //z2 = -z2;

        iprintf("\x1b[18;0H"
                "z1: %d   \n"
                "z2: %d   ",
                z1, z2);

        if(x2 < 0 || x1 > 7)
            return; // The ship has fallen off!

        if(x1 == x2 && z1 == z2) {
            out.push_back({z1, x1});
        }
        else if(x1 == x2) {
            out.push_back({z1, x1});
            out.push_back({z1, x2});
        }
        else if(z1 == z2) {
            out.push_back({z1, x1});
            out.push_back({z2, x1});
        }
        else {
            out.push_back({z1, x1});
            out.push_back({z2, x1});
            out.push_back({z1, x2});
            out.push_back({z2, x2});
        }
    }
    void make_bounds(vector2i grid_index, grid_t const& grid, arrayvec<aabb, 32>& ret) {
        int const row = grid_index.x;
        int const col = grid_index.y;
        if(row < 0 || row >= grid.size())
            return;
        if(col < 0 || col > 7)
            return;

        cell c = grid[row][col].data;

        if(!(c.flags & cell::geometry)) {
            return;
        }

        constexpr f32 block_size = f32(geometry::draw::block_size);
        constexpr f32 tile_height = f32(geometry::draw::tile_height);
        constexpr f32 short_height = f32(geometry::draw::short_height);

        f32 const base_height = geometry::draw::altitude_step * c.altitude;

        f32 const left = (f32(col) - f32(3.5)) * block_size;
        f32 const right = left + block_size;
        f32 const front = f32(-row) * block_size;
        f32 const back = front - block_size;

        if(c.flags & cell::tunnel) {
            f32 top = base_height;
            if(c.flags & cell::high) {
                top += block_size;
            }
            else {
                // the same regardless of whether
                // we have a low block on top of
                // the tunnel or not.
                top += tile_height + short_height;
            }

            f32 bottom = base_height;
            if(c.flags & cell::tile) {
                f32 const floor = base_height + tile_height;
                ret.push_back({{left, bottom, back}, {right, floor, front}});
            }
            else {
                bottom += tile_height;
            }

            f32 const ceiling = base_height + f32(geometry::tunnel::inner[3].y);

            ret.push_back({{left,bottom,back},{left+tile_height,ceiling,front}});
            ret.push_back({{left,ceiling,back},{right,top,front}});
            ret.push_back({{right-tile_height,bottom,back},{right,ceiling,front}});
        }
        else {
            f32 const bottom = base_height;
            f32 top = bottom;
            if(c.flags & cell::high) {
                top += block_size;
            }
            else if(c.flags & cell::low) {
                top += (tile_height + short_height);
            }
            else {
                // just a tile
                top += tile_height;
            }

            ret.push_back({{left, bottom, back}, {right, top, front}});
        }
    }

    sweep_result_t sweep_collide(aabb const& a, aabb const& b, vector3f32 const& velocity, int index) {
        vector3f32 overlap_start { -1024, -1024, -1024 }, overlap_end { 1024, 1024, 1024 };
        vector3f32 const one_by_velocity { f32(1) / velocity.x, f32(1) / velocity.y, f32(1) / velocity.z };

        for(size_t d = 0; d < 3; ++d) {
            // is [b] outside [a] and moving parallel or away?
            if(b.min[d] >= a.max[d] || a.min[d] >= b.max[d]) { 
                if(velocity[d] == f32(0))
                    return sweep::never {}; // can't collide; b is moving in parallel to a
                else if(b.min[d] > a.max[d] && velocity[d] > f32(0))
                    return sweep::never {}; // can't collide; b is moving away in a positive direction
                else if(a.min[d] > b.max[d] && velocity[d] < f32(0))
                    return sweep::never {}; // can't collide; b is moving away in a negative direction
            }

            // is [b] overlapping with [a]?
            if(b.min[d] < a.max[d] && a.min[d] < b.max[d]) {
                // this dimension is already overlapping
                overlap_start[d] = f32(-1);
                if(velocity[d] == f32(0)) {
                    // this dimension overlaps until infinity (on this trajectory)
                    overlap_end[d] = f32(INT_MAX, raw_tag);
                }
                else if(velocity[d] > f32(0)) {
                    // moving to the positive
                    overlap_end[d] = (a.max[d] - b.min[d]) * one_by_velocity[d];
                }
                else {
                    // moving to the negative
                    overlap_end[d] = (b.max[d] - a.min[d]) * (-one_by_velocity[d]);
                }
            }
            // is [b] approaching [a] from the negative side?
            else if(velocity[d] > f32(0)) {
                if(a.min[d] == b.max[d]) {
                    overlap_start[d] = f32(1, raw_tag); // "infinitesimal"
                }
                else {
                    overlap_start[d] = (a.min[d] - b.max[d]) * one_by_velocity[d];
                }
                overlap_end[d] = (a.max[d] - b.min[d]) * one_by_velocity[d];
            }
            // is [b] approaching [a] from the positive side?
            else { // if(velocity[d] < f32(0)) 
                if(b.min[d] == a.max[d]) {
                    overlap_start[d] = f32(1, raw_tag);
                }
                else {
                    overlap_start[d] = (b.min[d] - a.max[d]) * (-one_by_velocity[d]);
                }
                overlap_end[d] = (b.max[d] - a.min[d]) * (-one_by_velocity[d]);
            }
        }

        f32 start = std::max({overlap_start.x, overlap_start.y, overlap_start.z});

        //if(start == f32(0)) {
        //    // touching precisely but not overlapping
        //    return sweep::never {};
        //}

        if(start == f32(-1)) {
            return sweep::already { index };
        }

        f32 end = std::min({overlap_end.x, overlap_end.y, overlap_end.z});

        if(start > end || start >= f32(1)) {
            return sweep::never {};
        }
        else {
            // choose the last dimension to overlap
            dimension dim;
            if(start == overlap_start.x)
                dim = dimension::x;
            else if(start == overlap_start.y)
                dim = dimension::y;
            else
                dim = dimension::z;
            return sweep::from { start, index, dim };
        }
    }

    std::vector<aabb> last_bounds;
    aabb last_ship_bounds, last_next_ship_bounds;
    vector3f32 vel_prev0 { 0, 0, 0 }, vel_prev1 { 0, 0, 0 };


    collide_result_t collide(vector3f32 const& position, vector3f32 const& velocity, grid_t const& grid) {
        // for debugging
        vel_prev0 = vel_prev1;
        vel_prev1 = velocity;

        // let's just assume we never move more than two tiles in a single frame
        // because that simplifies things a lot
        using geometry::draw::ship_size;
        aabb const& ship_bounds { position, position + ship_size };

        // first find the nearby tiles
        arrayvec<vector2i, 8> tiles;
        select_tiles(position + velocity, tiles);
        if(tiles.size() == 0) {
            return collision::fell_off { };
        }
        select_tiles(position, tiles);
        std::sort(tiles.begin(), tiles.end());
        tiles.erase(std::unique(tiles.begin(), tiles.end()), tiles.end());

        // then get all the objects in those tiles; the maximum
        // is four aabbs for a tile with a tunnel in it, so for
        // eight tiles we have a maximum of 32 aabbs
        arrayvec<aabb, 32> bounds;
        for(vector2i const& v : tiles) {
            make_bounds(v, grid, bounds);
        }

        last_bounds.assign(bounds.begin(), bounds.end());
        last_ship_bounds = ship_bounds;
        last_next_ship_bounds.min = ship_bounds.min + velocity;
        last_next_ship_bounds.max = ship_bounds.max + velocity;

        // for each of those, we find a collision (if there is any)
        arrayvec<sweep_result_t, 32> sweep_results;
        size_t index = 0;
        for(aabb const& a : bounds) {
            sweep_results.push_back(sweep_collide(a, ship_bounds, velocity, index++));
        }

        if(sweep_results.size() == 0) {
            return collision::none {};
        }

        auto when = *std::min_element(sweep_results.begin(), sweep_results.end(), compare_sweep_result());
        return visit<collide_result_t>(when,
              [&bounds, &tiles, &ship_bounds](sweep::already a) -> collide_result_t {
                  return collision::already {
                      bounds[a.index],
                      {ship_bounds.min - vel_prev0, ship_bounds.max - vel_prev0},
                      tiles[a.index],
                      vel_prev0
                      };
              },
              [](sweep::never) { return collision::none {}; },
              [&velocity](sweep::from f) { return collision::correction { f.time, f.dim }; }
              );
    }
}
