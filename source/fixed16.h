#ifndef DSR_FIXED16_H
#define DSR_FIXED16_H

#include <boost/cstdint.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/mpl/not.hpp>

#include <nds/arm9/math.h>

namespace roads
{
    template <unsigned Frac> struct fixed16;
    struct raw_tag_t {};
    constexpr raw_tag_t raw_tag {};

    namespace detail
    {
        struct sfinae_tag_t {};
        constexpr sfinae_tag_t sfinae_tag {};

        template <bool P, unsigned From, unsigned To>
        struct convert_detail
        {
            static constexpr int16_t apply(int16_t from)
            {
                return from >> (From - To);
            }
        };

        template <unsigned From, unsigned To>
        struct convert_detail<false, From, To>
        {
            static constexpr int16_t apply(int16_t from)
            {
                return from << (To - From);
            }
        };

        template <unsigned FromFrac, unsigned ToFrac>
        int16_t constexpr convert(int16_t from)
        {
            return convert_detail<(FromFrac > ToFrac), FromFrac, ToFrac>::apply(from);
        }

        template <typename To, typename From, typename Enable = void>
        struct converter;

        template <typename T>
        struct converter<T, T> {
            static constexpr T apply(T from) {
                return from;
            }
        };
        template <unsigned Frac, typename T>
        struct converter<fixed16<Frac>, T, typename boost::enable_if<boost::is_floating_point<T>>::type> {
            static constexpr fixed16<Frac> apply(T from) {
                return fixed16<Frac>(static_cast<int16_t>(from * (1 << Frac)), raw_tag);
            }
        };
        template <unsigned Frac, typename T>
        struct converter<T, fixed16<Frac>, typename boost::enable_if<boost::is_floating_point<T>>::type> {
            static constexpr T apply(fixed16<Frac> from) {
                return T(from.raw_value) / (1 << Frac);
            }
        };
        template <unsigned Frac, typename T>
        struct converter<fixed16<Frac>, T, typename boost::enable_if<boost::is_integral<T>>::type> {
            static constexpr fixed16<Frac> apply(T from) {
                return fixed16<Frac>(detail::convert<0, Frac>(from), raw_tag);
            }
        };
        template <unsigned Frac, typename T>
        struct converter<T, fixed16<Frac>, typename boost::enable_if<boost::is_integral<T>>::type> {
            static constexpr T apply(fixed16<Frac> from) {
                return (from.raw_value) >> Frac;
            }
        };
    }

#define Where(x) \
    typename boost::enable_if<x, detail::sfinae_tag_t>::type = detail::sfinae_tag
#define RetWhere(x, T) \
    typename boost::enable_if<x, T>::type

    ////////////////////////////////////////////////////////////////////////////
    /////////////////////// FIXED-POINT SUM OPERATORS //////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    // fix + fix
    template <unsigned Frac>
    constexpr fixed16<Frac> operator+(fixed16<Frac> left, fixed16<Frac> right) {
        return fixed16<Frac>(left.raw_value + right.raw_value, raw_tag);
    }
    // fix + any
    template <unsigned Frac, typename Right>
    constexpr fixed16<Frac> operator+(fixed16<Frac> left, Right right) {
        return left + detail::converter<fixed16<Frac>, Right>::apply(right);
    }
    // any + fix
    template <unsigned Frac, typename Left>
    constexpr fixed16<Frac> operator+(Left left, fixed16<Frac> right) {
        return detail::converter<fixed16<Frac>, Left>::apply(left) + right;
    }
    

    ////////////////////////////////////////////////////////////////////////////
    /////////////////// FIXED-POINT DIFFERENCE OPERATORS ///////////////////////
    ////////////////////////////////////////////////////////////////////////////
    // fix - fix
    template <unsigned Frac>
    constexpr fixed16<Frac> operator-(fixed16<Frac> left, fixed16<Frac> right) {
        return fixed16<Frac>(left.raw_value - right.raw_value, raw_tag);
    }
    // fix - any
    template <unsigned Frac, typename Right>
    constexpr fixed16<Frac> operator-(fixed16<Frac> left, Right right) {
        return left - detail::converter<fixed16<Frac>, Right>::apply(right);
    }
    // any - fix
    template <typename Left, unsigned Frac>
    constexpr fixed16<Frac> operator-(Left left, fixed16<Frac> right) {
        return detail::converter<fixed16<Frac>, Left>::apply(left) - right;
    }


    ////////////////////////////////////////////////////////////////////////////
    ////////////////// FIXED-POINT MULTIPLICATION OPERATORS ////////////////////
    ////////////////////////////////////////////////////////////////////////////
    // fix * fix
    template <unsigned Frac>
    constexpr fixed16<Frac> operator*(fixed16<Frac> left, fixed16<Frac> right) {
        return fixed16<Frac>(int16_t((int32_t(left.raw_value) * right.raw_value) >> Frac), raw_tag);
    }
    // fix * int
    template <unsigned Frac, typename Right>
    constexpr
    typename boost::enable_if<boost::is_integral<Right>, fixed16<Frac>>::type
    operator*(fixed16<Frac> left, Right right) {
        return fixed16<Frac>(left.raw_value * right, raw_tag);
    }
    // int * fix
    template <typename Left, unsigned Frac>
    constexpr
    typename boost::enable_if<boost::is_integral<Left>, fixed16<Frac>>::type
    operator*(Left left, fixed16<Frac> right) {
        return fixed16<Frac>(left * right.raw_value, raw_tag);
    }
    // fix * float
    template <unsigned Frac, typename Right>
    constexpr
    typename boost::enable_if<boost::is_floating_point<Right>, fixed16<Frac>>::type
    operator*(fixed16<Frac> left, Right right) {
        return left * detail::converter<fixed16<Frac>, Right>::apply(right);
    }
    // float * fix
    template <typename Left, unsigned Frac>
    constexpr
    typename boost::enable_if<boost::is_floating_point<Left>, fixed16<Frac>>::type
    operator*(Left left, fixed16<Frac> right) {
        return detail::converter<fixed16<Frac>, Left>::apply(left) * right;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////// FIXED-POINT DIVISION OPERATORS //////////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    template <unsigned Frac>
    struct fixed16
    {
        int16_t raw_value;

        constexpr fixed16() : raw_value() {}

        //constexpr fixed16(fixed16 const& rhs) : raw_value(rhs.raw_value) {}

        template <unsigned OtherFrac>
        constexpr fixed16(fixed16<OtherFrac> rhs, typename boost::disable_if_c<Frac == OtherFrac>::type* = 0)
            : raw_value(detail::convert<OtherFrac, Frac>(rhs.raw_value)) {}

        template <typename T>
        constexpr fixed16(T rhs, Where(boost::is_integral<T>))
            : raw_value(detail::convert<0, Frac>(rhs))
        {
        }

        template <typename T>
        constexpr fixed16(T rhs, raw_tag_t) : raw_value(rhs) {
        }

        template <typename T>
        constexpr fixed16(T rhs, Where(boost::is_floating_point<T>))
            : raw_value(static_cast<int16_t>(rhs * (1 << Frac)))
        {
        }

        template <typename T>
        fixed16& operator+=(T rhs) {
            *this = (*this + rhs);
            return *this;
        }

        template <typename T>
        fixed16& operator-=(T rhs) {
            *this = (*this - rhs);
            return *this;
        }

        template <typename T>
        fixed16& operator*=(T rhs) {
            *this = (*this * rhs);
            return *this;
        }

        template <unsigned OtherFrac>
        fixed16& operator/=(fixed16<OtherFrac> rhs)
        {
            int32_t const tmp0 = int32_t(raw_value) << Frac;
            int32_t const tmp1 = int32_t(rhs.raw_value) >> (OtherFrac - Frac);
            raw_value = int16_t(div32(tmp0, tmp1));
            return *this;
        }

        template <typename T>
        RetWhere(boost::is_floating_point<T>, fixed16&)
        operator/=(T rhs)
        {
            *this /= fixed16(rhs);
            return *this;
        }

        template <typename T>
        RetWhere(boost::is_integral<T>, fixed16&)
        operator/=(T rhs)
        {
            int32_t const tmp0 = int32_t(raw_value) << Frac;
            int32_t const tmp1 = int32_t(rhs);
            raw_value = int16_t(div32(tmp0, tmp1));
            return *this;
        }

        template <typename T>
        RetWhere(boost::is_integral<T>, fixed16&)
        operator<<=(T rhs) {
            raw_value <<= rhs;
            return *this;
        }

        template <typename T>
        RetWhere(boost::is_integral<T>, fixed16&)
        operator>>=(T rhs) {
            raw_value >>= rhs;
            return *this;
        }

        constexpr fixed16 operator-()
        {
            return fixed16(-raw_value, raw_tag);
        }

        constexpr float to_float()
        {
            return float(raw_value) / (1 << Frac);
        }

        constexpr int to_int()
        {
            return int(raw_value) >> Frac;
        }
    };

    template <unsigned Lhs, unsigned Rhs>
    constexpr bool operator==(fixed16<Lhs> lhs, fixed16<Rhs> rhs)
    {
        return lhs.raw_value == fixed16<Lhs>(rhs).raw_value;
    }
    template <unsigned Lhs, unsigned Rhs>
    constexpr bool operator!=(fixed16<Lhs> lhs, fixed16<Rhs> rhs)
    {
        return !(lhs == rhs);
    }

    template <unsigned Lhs, unsigned Rhs>
    constexpr bool operator<(fixed16<Lhs> lhs, fixed16<Rhs> rhs)
    {
        return lhs.raw_value < fixed16<Lhs>(rhs).raw_value;
    }
    template <unsigned Lhs, unsigned Rhs>
    constexpr bool operator>(fixed16<Lhs> lhs, fixed16<Rhs> rhs)
    {
        return lhs.raw_value > fixed16<Lhs>(rhs).raw_value;
    }
    template <unsigned Lhs, unsigned Rhs>
    constexpr bool operator<=(fixed16<Lhs> lhs, fixed16<Rhs> rhs)
    {
        return lhs.raw_value <= fixed16<Lhs>(rhs).raw_value;
    }
    template <unsigned Lhs, unsigned Rhs>
    constexpr bool operator>=(fixed16<Lhs> lhs, fixed16<Rhs> rhs)
    {
        return lhs.raw_value >= fixed16<Lhs>(rhs).raw_value;
    }
    template <unsigned Lhs, unsigned Rhs>
    fixed16<Lhs> operator/(fixed16<Lhs> lhs, fixed16<Rhs> rhs)
    {
        fixed16<Lhs> ret(lhs);
        ret /= rhs;
        return ret;
    }
    template <unsigned Lhs, typename Rhs>
    fixed16<Lhs> operator/(fixed16<Lhs> lhs, Rhs rhs)
    {
        fixed16<Lhs> ret(lhs);
        ret /= rhs;
        return ret;
    }
    template <typename Lhs, unsigned Rhs>
    fixed16<Rhs> operator/(Lhs lhs, fixed16<Rhs> rhs)
    {
        fixed16<Rhs> ret(rhs);
        ret /= lhs;
        return ret;
    }
    template <unsigned Lhs, typename Rhs>
    constexpr fixed16<Lhs> operator<<(fixed16<Lhs> lhs, Rhs rhs) {
        return fixed16<Lhs>(lhs.raw_value << rhs, raw_tag);
    }
    template <unsigned Lhs, typename Rhs>
    constexpr fixed16<Lhs> operator>>(fixed16<Lhs> lhs, Rhs rhs) {
        return fixed16<Lhs>(lhs.raw_value >> rhs, raw_tag);
    }

    typedef fixed16<12> f16;

    template <unsigned N>
    constexpr fixed16<N> clamp(fixed16<N> val, fixed16<N> min, fixed16<N> max)
    {
        return (val < min) ? min : ((val > max) ? max : val);
    }

    template <unsigned Frac>
    constexpr fixed16<Frac> floor(fixed16<Frac> val) {
        return fixed16<Frac>(
            (
                (val.raw_value >> Frac) // discard fractional bits, make an integer
                - (val.raw_value < 0 ? 1 : 0) // if < 0 then subtract 1
                ) << Frac, // go back to fixed-point
            raw_tag);
    }

    template <unsigned Frac>
    constexpr fixed16<Frac> ceil(fixed16<Frac> val) {
        return fixed16<Frac>(
            (
                (val.raw_value >> Frac) // discard fractional bits, make an integer
                + (val.raw_value > 0 ? 1 : 0) // if > 0 then add 1
                ) << Frac, // go back to fixed-point
            raw_tag);
    }

    template <unsigned Frac>
    struct fixed32 {
        int32_t raw_value;
        constexpr fixed32() : raw_value(0) {}
        constexpr fixed32(int32_t int_value) : raw_value(int_value << Frac) {}
        constexpr fixed32(int32_t raw_value, raw_tag_t) : raw_value(raw_value) {}
        constexpr fixed32(float f_value) : raw_value(double(f_value) * (1 << Frac)) {}
        constexpr fixed32(double f_value) : raw_value(f_value * (1 << Frac)) {}
        template <unsigned RFrac>
        constexpr fixed32(fixed16<RFrac> f)
        : raw_value(
            (RFrac > Frac)
            ? (f.raw_value >> (RFrac - Frac)) // truncating
            : (f.raw_value << (Frac - RFrac)) // increasing precision
        ) {}
        fixed32& operator*=(fixed32 const& rhs) {
            raw_value = ((*this) * rhs).raw_value;
            return *this;
        }
        fixed32& operator+=(fixed32 const& rhs) {
            raw_value = ((*this) + rhs).raw_value;
            return *this;
        }
    };

    template <unsigned Frac>
    constexpr fixed32<Frac> operator+(fixed32<Frac> lhs, fixed32<Frac> rhs) {
        return fixed32<Frac>(lhs.raw_value + rhs.raw_value, raw_tag);
    }

    template <unsigned Frac>
    constexpr fixed32<Frac> operator-(fixed32<Frac> lhs, fixed32<Frac> rhs) {
        return fixed32<Frac>(lhs.raw_value - rhs.raw_value, raw_tag);
    }

    template <unsigned Frac>
    constexpr fixed32<Frac> operator*(fixed32<Frac> lhs, fixed32<Frac> rhs) {
        return fixed32<Frac>((int64_t(lhs.raw_value) * rhs.raw_value) >> Frac, raw_tag);
    }
}

#endif // DSR_FIXED16_H

#include "fixedvector.h"

