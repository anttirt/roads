#ifndef ROADS_COLLIDE_H
#define ROADS_COLLIDE_H

#include <boost/variant.hpp>
#include <vector>
#include "vector.h"
#include "fixed16.h"
#include "level.h"
#include "utility.h"

namespace roads {
    // axis-aligned bounding box
    struct aabb {
        vector3f32 min, max;
        bool operator==(aabb const& rhs) const { return min == rhs.min && max == rhs.max; }
    };

    typedef vector<int, 2> vector2i;
    namespace collision {
        struct already { aabb box, ship; vector2i tile; vector3f32 prev_velocity; bool operator==(already const& rhs) const { return box == rhs.box; } };
        struct correction { f32 time; dimension dim; bool operator==(correction const& rhs) const { return time == rhs.time && dim == rhs.dim; } };
        struct fell_off { bool operator==(fell_off) const { return true; } };
        struct none { bool operator==(none) const { return true; } };
    }

    extern std::vector<aabb> last_bounds;
    extern aabb last_ship_bounds;
    extern aabb last_next_ship_bounds;

    typedef boost::variant<collision::already, collision::correction, collision::none, collision::fell_off> collide_result_t;

    collide_result_t collide(vector3f32 const& position, vector3f32 const& velocity, grid_t const& grid);

    namespace sweep {
        struct already { size_t index; bool operator==(already r) const { return index == r.index; } };
        struct never { bool operator==(already) const {return true;}};
        struct from { f32 time; size_t index; dimension dim; bool operator==(from r) const { return time == r.time && index == r.index; } };
    }

    typedef boost::variant<sweep::already, sweep::never, sweep::from> sweep_result_t;

    // This assumes the first argument is stationary, but can easily be used
    // for two moving objects by transforming the variables into the reference
    // frame of the first object. The return value is always [0,1[ and represents
    // the fraction of the time step at which the collision occurred, or 1 if
    // there was no collision. You can get the value of b.min at the collision
    // by multiplying velocity by the return value.
    sweep_result_t sweep_collide(aabb const& a, aabb const& b, vector3f32 const& velocity, int index = 0);


}


#endif // ROADS_COLLIDE_H

