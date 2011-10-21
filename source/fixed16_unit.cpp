#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"
#include "fixed16.h"

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

    typedef fixed16<12> fix;

    namespace
    {
        UNIT_TEST(fixed_sum,
        {
            UASSERT_EQUAL(fix(5) - fix(3), fix(2));
            UASSERT_EQUAL(fix(-1.f / 16) + fix(1.f / 8), fix(1.f / 16));
            UASSERT_EQUAL(fix(0.25f) + 3, fix(3.25f));
            UASSERT_EQUAL(fix(-2.f) + 4.f, fix(2));
        });

        UNIT_TEST(fixed_mul,
        {
            UASSERT_EQUAL(fix(4) * fix(0.25f), fix(1.f));
            UASSERT_EQUAL(fix(0.25f) * fix(-0.5f), fix(-.125f));
        });

        UNIT_TEST(fixed_convert,
        {
            UASSERT_EQUAL(fix(0.3f).raw_value, floattov16(0.3f));
        });

        template <typename T>
        std::auto_ptr<unit_test_base> make_auto(T* p) { return std::auto_ptr<unit_test_base>(p); }
    }

    template <>
    void create_tests<fixed16<12> >(unit_test_suite& suite)
    {
        suite.add_test(make_auto(new fixed_sum));
        suite.add_test(make_auto(new fixed_mul));
        suite.add_test(make_auto(new fixed_convert));
    }
}

#endif
