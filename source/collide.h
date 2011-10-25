#ifndef ROADS_COLLIDE_H
#define ROADS_COLLIDE_H

#include <boost/variant.hpp>
#include <vector>
#include "vector.h"
#include "fixed16.h"
#include "level.h"

namespace roads {
    typedef vector<int, 2> vector2i;
    namespace collision {
        struct correction { vector3f32 offset; };
        struct fell_off { };
        struct none { };
    }

    // axis-aligned bounding box
    struct aabb {
        vector3f32 min, max;
    };

    extern std::vector<aabb> last_bounds;

    typedef boost::variant<collision::correction, collision::none, collision::fell_off> collide_result_t;

    collide_result_t collide(vector3f32 const& position, vector3f32 const& velocity, grid_t const& grid);
}


#endif // ROADS_COLLIDE_H

