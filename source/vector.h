#ifndef DSR_VECTOR_H_INCLUDED
#define DSR_VECTOR_H_INCLUDED

namespace roads
{
    template <typename T, int Stage>
    struct sqrt_stage {
        static constexpr T apply(T x, T root) {
            return (root * root - x == T(0))
                ? root
                : sqrt_stage<T, Stage - 1>::apply(x, (root + x/root)/2);
        }
    };
    
    template <typename T>
    struct sqrt_stage<T, 0> {
        static constexpr T apply(T x, T root) {
            return root;
        }
    };

    template <typename T>
    constexpr T csqrt(T x) {
        return sqrt_stage<T, 5>::apply(x, x / 2);
    }

    template <typename Elem, unsigned N> struct vector;
    template <typename Elem> struct vector<Elem, 2> {
        Elem x, y;
        constexpr vector() = default;
        constexpr vector(Elem x, Elem y)
            : x(x), y(y) {}

        constexpr Elem get(int i) { return i == 0 ? x : y; }

        template <typename E2>
        constexpr vector<E2, 2> convert() { return vector<E2, 2>(x, y); }

        constexpr vector operator-() { return vector(-x, -y); }

        constexpr vector scale(vector s) {
            return vector(x * s.x, y * s.y);
        }

        constexpr Elem length() {
            return csqrt(x * x + y * y);
        }

        Elem& operator[](size_t ix) {
            return ix == 0 ? x : y;
        }
        constexpr Elem operator[](size_t ix) const {
            return ix == 0 ? x : y;
        }

        constexpr vector normalized() {
            return vector(x / length(), y / length());
        }

        static constexpr vector unitx() { return vector(Elem(1), 0); }
        static constexpr vector unity() { return vector(0, Elem(1)); }

        vector& operator+=(vector const& rhs) { x += rhs.x; y += rhs.y; return *this; }
        vector& operator-=(vector const& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
        template <typename Other>
        vector& operator*=(Other b) { x *= b; y *= b; return *this; }
    };

    static_assert(std::is_trivial<vector<int, 2>>::value, "vector2 type is not trivial");
    static_assert(std::is_standard_layout<vector<int, 2>>::value, "vector2 type is not standard-layout");

    template <typename Elem> struct vector<Elem, 3> {
        Elem x, y, z;
        constexpr vector() = default;
        constexpr vector(Elem x, Elem y, Elem z)
            : x(x), y(y), z(z) {}

        constexpr Elem get(int i) { return i == 0 ? x : i == 1 ? y : z; }

        template <typename E2>
        constexpr vector<E2, 3> convert() { return vector<E2, 3>(x, y, z); }

        constexpr vector operator-() { return vector(-x, -y, -z); }

        constexpr vector scale(vector s) {
            return vector(x * s.x, y * s.y, z * s.z);
        }

        constexpr Elem length() {
            return csqrt(x * x + y * y + z * z);
        }

        Elem& operator[](size_t ix) {
            return ix == 0 ? x : ix == 1 ? y : z;
        }
        constexpr Elem operator[](size_t ix) const {
            return ix == 0 ? x : ix == 1 ? y : z;
        }
        constexpr vector normalized() {
            return vector(x / length(), y / length(), z / length());
        }
        static constexpr vector unitx() { return vector(Elem(1), 0, 0); }
        static constexpr vector unity() { return vector(0, Elem(1), 0); }
        static constexpr vector unitz() { return vector(0, 0, Elem(1)); }

        vector& negate() { x = -x; y = -y; z = -z; return *this; }
        vector& operator+=(vector const& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        vector& operator-=(vector const& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        template <typename Other>
        vector& operator*=(Other b) { x *= b; y *= b; z *= b; return *this; }
    };

    static_assert(std::is_trivial<vector<int, 3>>::value, "vector2 type is not trivial");
    static_assert(std::is_standard_layout<vector<int, 3>>::value, "vector2 type is not standard-layout");

    template <typename Elem> inline constexpr vector<Elem, 2> operator+(vector<Elem, 2> a, vector<Elem, 2> b) {
        return vector<Elem, 2>(a.x + b.x, a.y + b.y);
    }
    template <typename Elem> inline constexpr vector<Elem, 3> operator+(vector<Elem, 3> a, vector<Elem, 3> b) {
        return vector<Elem, 3>(a.x + b.x, a.y + b.y, a.z + b.z);
    }
    template <typename Elem> inline constexpr vector<Elem, 2> operator-(vector<Elem, 2> a, vector<Elem, 2> b) {
        return vector<Elem, 2>(a.x - b.x, a.y - b.y);
    }
    template <typename Elem> inline constexpr vector<Elem, 3> operator-(vector<Elem, 3> a, vector<Elem, 3> b) {
        return vector<Elem, 3>(a.x - b.x, a.y - b.y, a.z - b.z);
    }
    template <typename Elem, typename Other> inline constexpr vector<Elem, 2> operator*(vector<Elem, 2> a, Other b) {
        return vector<Elem, 2>(a.x * b, a.y * b);
    }
    template <typename Elem, typename Other> inline constexpr vector<Elem, 3> operator*(vector<Elem, 3> a, Other b) {
        return vector<Elem, 3>(a.x * b, a.y * b, a.z * b);
    }

    template <typename Elem> inline constexpr Elem dot(vector<Elem, 2> a, vector<Elem, 2> b) {
        return a.x * b.x + a.y * b.y;
    }
    template <typename Elem> inline constexpr Elem dot(vector<Elem, 3> a, vector<Elem, 3> b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    template <typename Elem> inline constexpr vector<Elem, 3> cross(vector<Elem, 3> a, vector<Elem, 3> b) {
        return vector<Elem, 3>(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
        );
    }

    template <typename Elem> inline constexpr bool operator==(vector<Elem, 2> a, vector<Elem, 2> b) {
        return a.x == b.x && a.y == b.y;
    }
    template <typename Elem> inline constexpr bool operator==(vector<Elem, 3> a, vector<Elem, 3> b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
    template <typename Elem, unsigned N> inline constexpr bool operator!=(vector<Elem, N> a, vector<Elem, N> b) {
        return !(a == b);
    }

    template <typename Elem>
    constexpr bool operator<(vector<Elem, 2> const& l, vector<Elem, 2> const& r) {
        return
            l.x < r.x ? true :
            (l.x > r.x ? false :
             (l.y < r.y));
    }

    template <typename Elem>
    constexpr bool operator<(vector<Elem, 3> const& l, vector<Elem, 3> const& r) {
        return
            l.x < r.x ? true :
            (l.x > r.x ? false :
             (l.y < r.y ? true :
              (l.y > r.y ? false :
               (l.z < r.z))));
    }

    template <typename Elem, unsigned N>
    constexpr bool operator>=(vector<Elem, N> const& l, vector<Elem, N> const& r) {
        return !(l < r);
    }
    template <typename Elem, unsigned N>
    constexpr bool operator>(vector<Elem, N> const& l, vector<Elem, N> const& r) {
        return r < l;
    }
    template <typename Elem, unsigned N>
    constexpr bool operator<=(vector<Elem, N> const& l, vector<Elem, N> const& r) {
        return !(r < l);
    }

    typedef vector<double, 2> vector2d;
    typedef vector<double, 3> vector3d;
}

#endif // DSR_VECTOR_H_INCLUDED

#include "fixedvector.h"
#include "cellvector.h"

