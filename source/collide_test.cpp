#include <boost/lexical_cast.hpp>
#include <sstream>
#include <ostream>

#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "collide.h"
#include "variant_access.hpp"
#include <nds.h>

namespace boost {
    template<>
    std::string lexical_cast<std::string>(roads::sweep_result_t const& s) {
        using namespace roads;
        using namespace sweep;
        std::stringstream ss;
        visit<void>(s,
            [&ss](already l) { ss << "sweep[already=" << l.index << "]"; },
            [&ss](from f) { ss << "sweep[from=" << f.time.to_float() << "]"; },
            [&ss](never) { ss << "sweep[never]"; }
            );
        return ss.str();
    }
}

namespace roads {
    namespace {
        std::ostream& operator<<(std::ostream& o, f32 f) {
            return o << f.to_float();
        }



        bool operator==(sweep_result_t const& l, sweep_result_t const& r) {
            using namespace roads;
            using namespace sweep;
            return visit_binary<bool>(l, r,
                        [](already l, already r) { return l.index == r.index; },
                        [](already, from) { return false; },
                        [](already, never) { return false; },
                        [](from, already) { return false; },
                        [](from l, from r) { return l.time == r.time; },
                        [](from, never) { return false; },
                        [](never, already) { return false; },
                        [](never, from) { return false; },
                        [](never, never) { return true; }
                        );
        }

        void check(aabb const& a, aabb const& b, vector3f32 const& v, sweep_result_t const& r) {
            auto sweep = sweep_collide(a, b, v, 0);
            UASSERT_EQUAL(sweep, r);
        }

        UNIT_TEST(test_sweep1,
        {
            check({ { 0, 0, 0 }, { 1, 1, 1 } }, { { 2, 2, 2 }, { 3, 3, 3 } }, { -4, -4, -4 }, sweep::from { f32(0.25), 0 });
            check({ { 0, 0, 0 }, { 1, 1, 1 } }, { { 2, 2, 2 }, { 3, 3, 3 } }, { -4, -4, 4 }, sweep::never {});
        });
        UNIT_TEST(test_sweep2,
        {
            check({ { 0, 0, 0 }, { 1, 1, 1 } }, { { -2, -2, -2 }, { -3, -3, -3 } }, { 4, 4, 4 }, sweep::from { f32(0.75), 0});
            check({ { 0, 0, 0 }, { 1, 1, 1 } }, { { 2, 2, 2 }, { 3, 3, 3 } }, { -0.5, -0.5, -0.5 }, sweep::never {});
            check({ { 0, 0, 0 }, { 1, 1, 1 } }, { { 0.5, 0.5, 0.5 }, { 1.5, 1.5, 1.5 } }, { -0.5, -0.5, -0.5 }, sweep::already { 0 });
        });
        UNIT_TEST(test_sweep_tile,
        {
            check({ { 0, 0, 0 }, { 6, 1, 6 } }, { { 2., 1.5, 2. }, { 4., 3.5, 4 } }, { 4, 0, 4 }, sweep::never {});
            check({ { 0, 0, 0 }, { 6, 1, 6 } }, { { 2., 1.5, 2. }, { 4., 3.5, 4 } }, { 4, -1, 4 }, sweep::from { f32(0.5), 0 });
        });
        UNIT_TEST(test_sweep_tile2,
        {
            check({ { 0, 0, 0 }, { 6, 1, 6 } }, { { 2., 1, 2. }, { 4., 3.5, 4 } }, { 4, 0, 4 }, sweep::never {});
        });
        UNIT_TEST(test_sweep_bug0,
        {
            check(
                { { f32(128, raw_tag), f32(0, raw_tag), f32(-14080, raw_tag) }, { f32(384, raw_tag), f32(42, raw_tag), f32(-13824, raw_tag) } },
                { { f32(48, raw_tag), f32(74, raw_tag), f32(-14038, raw_tag) }, { f32(176, raw_tag), f32(138, raw_tag), f32(-13910, raw_tag) } },
                { f32(8, raw_tag), f32(-48, raw_tag), f32(-34, raw_tag) }, sweep::from { f32(2./3.), 0 });
        });
        UNIT_TEST(test_sweep_bug1,
        {
            check(
                { { f32(128, raw_tag), f32(152, raw_tag), f32(-10496, raw_tag) }, { f32(384, raw_tag), f32(194, raw_tag), f32(-10240, raw_tag) } },
                { { f32(112, raw_tag), f32(195, raw_tag), f32(-10444, raw_tag) }, { f32(240, raw_tag), f32(259, raw_tag), f32(-10316, raw_tag) } },
                { f32(-8, raw_tag), f32(-6, raw_tag), f32(-30, raw_tag) }, sweep::from { f32(1./6.), 0 });
        });
        UNIT_TEST(test_sweep_bug2,
        {
            check(
                { { f32(128, raw_tag), f32(152, raw_tag), f32(-9472, raw_tag) }, { f32(384, raw_tag), f32(194, raw_tag), f32(-9216, raw_tag) } },
                { { f32(152, raw_tag), f32(194, raw_tag), f32(-9364, raw_tag) }, { f32(280, raw_tag), f32(258, raw_tag), f32(-9236, raw_tag) } },
                { f32(8, raw_tag), f32(-34, raw_tag), f32(-30, raw_tag) }, sweep::from { f32(1, raw_tag), 0 });
        });
        UNIT_TEST(test_sweep_bug3,
        {
            check(
                { { f32(128, raw_tag), f32(152, raw_tag), f32(-10240, raw_tag) }, { f32(384, raw_tag), f32(194, raw_tag), f32(-9984, raw_tag) } },
                { { f32(112, raw_tag), f32(197, raw_tag), f32(-10114, raw_tag) }, { f32(240, raw_tag), f32(261, raw_tag), f32(-9986, raw_tag) } },
                { f32(8, raw_tag), f32(-11, raw_tag), f32(-30, raw_tag) }, sweep::from { f32(1, raw_tag), 0 });
        });

        template <typename T>
        std::auto_ptr<unit_test_base> make_auto(T* p) { return std::auto_ptr<unit_test_base>(p); }
    }

    template <>
    void create_tests<collide_result_t>(unit_test_suite& suite)
    {
        suite.add_test(make_auto(new test_sweep1));
        suite.add_test(make_auto(new test_sweep2));
        suite.add_test(make_auto(new test_sweep_tile));
        suite.add_test(make_auto(new test_sweep_tile2));
        suite.add_test(make_auto(new test_sweep_bug0));
        suite.add_test(make_auto(new test_sweep_bug1));
        suite.add_test(make_auto(new test_sweep_bug2));
        suite.add_test(make_auto(new test_sweep_bug3));
    }
}

#endif


