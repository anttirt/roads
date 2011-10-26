#ifndef ROADS_VARIANT_ACCESS_HPP
#define ROADS_VARIANT_ACCESS_HPP

#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace roads {
    namespace detail {
        template <typename T> T fake();
        template <typename T>
        struct const_ {
            static bool const value = true;
        };

        template <typename R, typename... F>
        struct generated_visitor;

        template <typename R, typename F, typename... Fs>
        struct generated_visitor<R, F, Fs...> : generated_visitor<R, Fs...> {
            using generated_visitor<R, Fs...>::operator();
            F f;
            template <typename T>
            typename std::enable_if<const_<decltype(f(fake<T>()))>::value , R>::type
            operator()(T&& t) {
                return f(std::forward<T>(t));
            }
            generated_visitor(F&& f, Fs&&... fs)
                : generated_visitor<R, Fs...>(std::forward<Fs>(fs)...), f(std::forward<F>(f))
            {
            }
        };

        template <typename R, typename F>
        struct generated_visitor<R, F> {
            typedef R result_type;
            F f;
            template <typename T>
            typename std::enable_if<const_<decltype(f(fake<T>()))>::value , R>::type
            operator()(T&& t) {
                return f(std::forward<T>(t));
            }
            generated_visitor(F&& f)
                : f(std::forward<F>(f))
            {
            }
        };
        template <typename R, typename... F>
        struct generated_binary_visitor;

        template <typename R, typename F, typename... Fs>
        struct generated_binary_visitor<R, F, Fs...> : generated_binary_visitor<R, Fs...> {
            using generated_binary_visitor<R, Fs...>::operator();
            F f;
            template <typename T, typename U>
            typename std::enable_if<const_<decltype(f(fake<T>(), fake<U>()))>::value, R>::type
            operator()(T&& t, U&& u) {
                return f(std::forward<T>(t), std::forward<U>(u));
            }
            generated_binary_visitor(F&& f, Fs&&... fs)
                : generated_binary_visitor<R, Fs...>(std::forward<Fs>(fs)...), f(std::forward<F>(f))
            {
            }
        };

        template <typename R, typename F>
        struct generated_binary_visitor<R, F> {
            typedef R result_type;
            F f;
            template <typename T, typename U>
            typename std::enable_if<const_<decltype(f(fake<T>(), fake<U>()))>::value , R>::type
            operator()(T&& t, U&& u) {
                return f(std::forward<T>(t), std::forward<U>(u));
            }
            generated_binary_visitor(F&& f)
                : f(std::forward<F>(f))
            {
            }
        };
    }

    template <typename R, typename Variant, typename... Fs>
    R visit(Variant&& variant, Fs&&... fs) {
        auto visitor = detail::generated_visitor<R, Fs...>(std::forward<Fs>(fs)...);
        return boost::apply_visitor(visitor, variant);
    }
    template <typename R, typename V0, typename V1, typename... Fs>
    R visit_binary(V0&& v0, V1&& v1, Fs&&... fs) {
        auto visitor = detail::generated_binary_visitor<R, Fs...>(std::forward<Fs>(fs)...);
        return boost::apply_visitor(visitor, v0, v1);
    }
}

#endif // ROADS_VARIANT_ACCESS_HPP

