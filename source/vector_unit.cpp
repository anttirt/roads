#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "vector.h"

#include <boost/lexical_cast.hpp>
#include <nds.h>

namespace roads
{
    template <unsigned N>
    std::ostream& operator<<(std::ostream& os, fixed16<N> f)
    {
        os << (float(f.raw_value) / (1 << N));
        return os;
    }
    std::ostream& operator<<(std::ostream& os, vector3 v)
    {
        os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
        return os;
    }


    namespace
    {
        UNIT_TEST(vector_sum,
        {
            UASSERT_EQUAL(vec(1.f, 2, 0.25) + vec(-1.5f, -3, -0.125), vec(-.5f, -1, 0.125));
        });

        UNIT_TEST(vector_mul,
        {
            UASSERT_EQUAL(dot(vec(-2, 0.25), vec(0.5f, 2)), f16(-0.5));
        });

        template <typename T>
        std::auto_ptr<unit_test_base> make_auto(T* p) { return std::auto_ptr<unit_test_base>(p); }
    }

    template <>
    void create_tests<vector3>(unit_test_suite& suite)
    {
        suite.add_test(make_auto(new vector_sum));
        suite.add_test(make_auto(new vector_mul));
    }
}

#endif
