#include "collide.h"

#include <algorithm>
#include "arrayvec.hpp"
#include "geometry.h"

#include "disp_writer.h"

namespace roads {
    void select_tiles(vector3f32 const& position, arrayvec<vector2i, 8>& out);
    void make_bounds(vector2i position, grid_t const& grid, arrayvec<aabb, 32>& ret);

    // This assumes the first argument is stationary, but can easily be used
    // for two moving objects by transforming the variables into the reference
    // frame of the first object. The return value is always [0,1[ and represents
    // the fraction of the time step at which the collision occurred, or 1 if
    // there was no collision. You can get the value of b.min at the collision
    // by multiplying velocity by the return value.
    f32 sweep_collide(aabb const& a, aabb const& b, vector3f32 const& velocity);


    void select_tiles(vector3f32 const& position, arrayvec<vector2i, 8>& out) {
        f32 const xoff = geometry::draw::ship_size.x;
        f32 const zoff = geometry::draw::ship_size.z;
        f32 const block_size = f32(geometry::draw::block_size);

        int x1 = floor(f32(3.5) + (position.x       ) / block_size).to_int();
        int x2 = floor(f32(3.5) + (position.x + xoff) / block_size).to_int();
        int z1 = floor(          -(position.z - zoff) / block_size).to_int();
        int z2 = floor(          -(position.z       ) / block_size).to_int();

        //z1 = -z1;
        //z2 = -z2;

        iprintf("\x1b[18;0H"
                "z1: %d   \n"
                "z2: %d   ",
                z1, z2);

        if(x2 < 0 || x1 > 7)
            return; // The ship has fallen off!

        if(x1 == x2 && z1 == z2) {
            out.push_back({x1, z1});
        }
        else if(x1 == x2) {
            out.push_back({x1, z1});
            out.push_back({x1, z2});
        }
        else if(z1 == z2) {
            out.push_back({x1, z1});
            out.push_back({x2, z1});
        }
        else {
            out.push_back({x1, z1});
            out.push_back({x2, z1});
            out.push_back({x1, z2});
            out.push_back({x2, z2});
        }
    }
    void make_bounds(vector2i grid_index, grid_t const& grid, arrayvec<aabb, 32>& ret) {
        if(grid_index.y < 0 || grid_index.y >= grid.size())
            return;
        if(grid_index.x < 0 || grid_index.x > 7)
            return;

        cell c = grid[grid_index.y][grid_index.x].data;

        if(!(c.flags & cell::geometry)) {
            return;
        }

        constexpr f32 block_size = f32(geometry::draw::block_size);
        constexpr f32 tile_height = f32(geometry::draw::tile_height);
        constexpr f32 short_height = f32(geometry::draw::short_height);

        f32 const base_height = block_size * f32(0.5) * c.altitude;

        f32 const left = (f32(grid_index.x) - f32(3.5)) * block_size;
        f32 const right = left + block_size;
        f32 const front = f32(-grid_index.y) * block_size;
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
    f32 sweep_collide(aabb const& a, aabb const& b, vector3f32 const& velocity) {
        vector3f32 overlap_start, overlap_end;
        vector3f32 const one_by_velocity { f32(1) / velocity.x, f32(1) / velocity.y, f32(1) / velocity.z };

        for(size_t d = 0; d < 3; ++d) {
            if(a.max[d] < b.min[d] && velocity[d] < f32(0)) {
                overlap_start[d] = (a.max[d] - b.min[d]) * one_by_velocity[d];
            }
            else if(b.max[d] < a.min[d] && velocity[d] > f32(0)) {
                overlap_start[d] = (a.min[d] - b.max[d]) * one_by_velocity[d];
            }
            if(b.max[d] > a.min[d] && velocity[d] < f32(0)) {
                overlap_end[d] = (a.min[d] - b.max[d]) * one_by_velocity[d];
            }
            else if(a.max[d] > b.min[d] && velocity[d] > f32(0)) {
                overlap_end[d] = (a.max[d] - b.min[d]) * one_by_velocity[d];
            }
        }

        f32 start = std::max({overlap_start.x, overlap_start.y, overlap_start.z});
        f32 end = std::min({overlap_end.x, overlap_end.y, overlap_end.z});

        if(start > end) {
            return f32(1);
        }
        else {
            return start;
        }
    }

    std::vector<aabb> last_bounds;

    collide_result_t collide(vector3f32 const& position, vector3f32 const& velocity, grid_t const& grid) {
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

        // for each of those, we find a collision (if there is any)
        arrayvec<f32, 32> collisions;
        for(aabb const& a : bounds) {
            collisions.push_back(sweep_collide(a, ship_bounds, velocity));
        }

        int ccount = 0;
        for(f32 time : collisions) {
            if(time != f32(1))
                ++ccount;
        }

        iprintf("\x1b[10;2H"
                "tiles: %d      \n"
                "bounds: %d     \n"
                "colls: %d      \n",
                tiles.size(),
                bounds.size(),
                ccount
                );

        // then we find the earliest collision
        f32 earliest = 1;
        for(f32 time : collisions) {
            time = std::min(time, earliest);
        }

        // this means all collision checks reported 1 ie. no collision
        if(earliest == f32(1)) {
            return collision::none {};
        }
        
        // otherwise we correct the earliest collision, which automatically
        // disqualifies all the other collisions.
        return collision::correction { velocity * earliest };
    }
}
