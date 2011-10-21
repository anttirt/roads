#ifndef DSR_MATHCORE_H
#define DSR_MATHCORE_H

#include "address_transform.h"

namespace roads
{
    enum { math_address_base = 0x04000000 };

    enum math_offset_t
    {
        div_cr = 0x280,
        div_numerator = 0x290,
        div_denominator = 0x298,
        div_result = 0x2a0,
        div_remainder = 0x2a8,

        sqrt_cr = 0x2b0,
        sqrt_result = 0x2b4,
        sqrt_param = 0x2b8,
    };

    namespace detail
    {

    }

}


#endif // DSR_MATHCORE_H

