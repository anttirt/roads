#include "unit_config.h"

#if RUN_UNIT_TESTS == 1

#include "unit_test.h"

#include <algorithm>
#include <cstdio>
#include <cstdarg>

namespace roads
{
    void unit_test_suite::run_tests()
    {
        typedef std::vector<unit_test_base*> testvec;
        testvec::const_iterator begin = m_tests.begin(), end = m_tests.end();

        for(;begin != end; ++begin)
        {
            unit_test_base& test = **begin;

            try
            {
                test.run();
                iprintf("[SUCCESS: %s]\n", test.name().c_str());
            }
            catch(unit_test_failure& failure)
            {
                iprintf("[FAILED: %s]\n\tReason: %s\n", test.name().c_str(), failure.what());
            }
        }
    }
    unit_test_suite::~unit_test_suite()
    {
        std::transform(m_tests.begin(), m_tests.end(), m_tests.begin(), deleter());
    }

    void throw_on_fail(bool success, char const* msg, ...)
    {
        if(!success)
        {
		    char messageBuffer[1024];
		    {
			    va_list args;
			    va_start(args, msg);
			    vsnprintf(messageBuffer, 1024, msg, args);
			    va_end(args);
		    }

            throw roads::unit_test_failure(messageBuffer);
        }
    }

}

#endif
