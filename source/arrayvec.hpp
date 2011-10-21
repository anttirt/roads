#ifndef ARRAYVEC_HPP
#define ARRAYVEC_HPP

#include <iterator>
#include <cstddef>
#include <cassert>
#include <stdexcept>

namespace avpolicy {
    struct dummyst { virtual void f() = 0; };
    template <typename, size_t, template <typename> class, template <typename> class> struct arrayvec;
    template <typename> struct throw_on_overflow;
    template <typename T, int Max, template <typename> class Under>
    struct throw_on_overflow<arrayvec<T, Max, throw_on_overflow, Under>> {
        void push_back(T const& t) {
            arrayvec<T, Max, ::avpolicy::throw_on_overflow, Under>* derived = static_cast<arrayvec<T, Max, ::avpolicy::throw_on_overflow, Under>*>(this);
            if(derived->liveCount >= Max) {
                throw std::out_of_range("");
            }
            new (derived->get(derived->liveCount)) T(t);
            ++(derived->liveCount);
        }
    };
    template <typename> struct assert_on_overflow;
    template <typename T, int Max, template <typename> class Under>
    struct assert_on_overflow<arrayvec<T, Max, assert_on_overflow, Under>> {
        void push_back(T const& t) {
            arrayvec<T, Max, ::avpolicy::assert_on_overflow, Under>* derived = static_cast<arrayvec<T, Max, ::avpolicy::assert_on_overflow, Under>*>(this);
            assert(derived->liveCount < Max);
            new (derived->get(derived->liveCount)) T(t);
            ++(derived->liveCount);
        }
    };
    template <typename> struct false_on_overflow;
    template <typename T, int Max, template <typename> class Under>
    struct false_on_overflow<arrayvec<T, Max, false_on_overflow, Under>> {
        bool push_back(T const& t) {
            arrayvec<T, Max, ::avpolicy::false_on_overflow, Under>* derived = static_cast<arrayvec<T, Max, ::avpolicy::false_on_overflow, Under>*>(this);
            if(derived->liveCount >= Max)
                return false;
            new (derived->get(derived->liveCount)) T(t);
            ++(derived->liveCount);
            return true;
        }
    };
    template <typename> struct throw_on_underflow;
    template <typename T, int Max, template <typename> class Over>
    struct throw_on_underflow<arrayvec<T, Max, Over, throw_on_underflow>> {
        void pop_back() {
            arrayvec<T, Max, Over, ::avpolicy::throw_on_underflow>* derived = static_cast<arrayvec<T, Max, Over, ::avpolicy::throw_on_underflow>*>(this);
            if(derived->liveCount < 1) {
                throw std::out_of_range("");
            }
            derived->get(derived->liveCount - 1)->~T();
            --(derived->liveCount);
        }
    };
    template <typename> struct assert_on_underflow;
    template <typename T, int Max, template <typename> class Over>
    struct assert_on_underflow<arrayvec<T, Max, Over, assert_on_underflow>> {
        void pop_back() {
            arrayvec<T, Max, Over, ::avpolicy::assert_on_underflow>* derived = static_cast<arrayvec<T, Max, Over, ::avpolicy::assert_on_underflow>*>(this);
            assert(derived->liveCount > 0);
            derived->get(derived->liveCount - 1)->~T();
            --(derived->liveCount);
        }
    };
    template <typename> struct false_on_underflow;
    template <typename T, int Max, template <typename> class Over>
    struct false_on_underflow<arrayvec<T, Max, Over, false_on_underflow>> {
        bool pop_back() {
            arrayvec<T, Max, Over, ::avpolicy::false_on_underflow>* derived = static_cast<arrayvec<T, Max, Over, ::avpolicy::false_on_underflow>*>(this);
            if(derived->liveCount < 1)
                return false;
            derived->get(derived->liveCount - 1)->~T();
            --(derived->liveCount);
            return true;
        }
    };

    // A linear container of up to Max elements of type T stored entirely
    // in a single contiguous memory location, without any dynamic allocation.
    //
    //
    // Exception safety
    // ----------------
    //
    // All guarantees require ~T() to be no-throw.
    //
    // If T() and T(T const&) are no-throw, all arrayvec operations are no-throw, except for the following:
    // push_back(T const&): see OverflowPolicy
    // pop_back(): see UnderflowPolicy
    //
    // If T() and T(T const&) can throw, the following guarantees apply:
    // arrayvec(): no-throw
    // arrayvec(size_t): strong
    // arrayvec(size_t, T const&): strong
    // arrayvec(ForwardIterator, ForwardIterator): strong
    // arrayvec(arrayvec const&): strong
    // arrayvec& operator=(arrayvec const&): basic
    template <typename T, size_t Max, template <typename> class OverflowPolicy = throw_on_overflow, template <typename> class UnderflowPolicy = throw_on_underflow>
    struct arrayvec : OverflowPolicy<arrayvec<T, Max, OverflowPolicy>> {
        friend struct OverflowPolicy<arrayvec<T, Max, OverflowPolicy, UnderflowPolicy>>;
        friend struct UnderflowPolicy<arrayvec<T, Max, OverflowPolicy, UnderflowPolicy>>;

        typedef T value_type;
        typedef T* iterator;
        typedef T const* const_iterator;

        arrayvec() : liveCount(0) {}
        arrayvec(size_t n) : liveCount(n) {
            assert(liveCount <= Max);
            for(size_t i = 0; i < n; ++i) {
                try {
                    new (get(i)) T();
                }
                catch(...) {
                    for(size_t j = 0; j < i; ++j) {
                        get(j)->~T();
                    }
                    throw;
                }
            }
        }
        arrayvec(size_t n, T const& x) : liveCount(n) {
            assert(liveCount <= Max);
            for(size_t i = 0; i < n; ++i) {
                try {
                    new (get(i)) T(x);
                }
                catch(...) {
                    for(size_t j = 0; j < i; ++j) {
                        get(j)->~T();
                    }
                    throw;
                }
            }
        }
        template <typename ForwardIterator>
            arrayvec(ForwardIterator b, ForwardIterator e) : liveCount(std::distance(b, e)) {
                assert(liveCount <= Max);
                construct(b, e);
            }
        arrayvec(arrayvec const& rhs) : liveCount(rhs.liveCount) {
            construct(rhs.begin(), rhs.end());
        }
        arrayvec& operator=(arrayvec const& rhs) {
            destroy();
            construct(rhs.begin(), rhs.end());
            liveCount = rhs.liveCount;
        }
        ~arrayvec() {
            destroy();
        }

        iterator begin() { return get(0); }
        iterator end() { return get(liveCount); }
        const_iterator begin() const { return get(0); }
        const_iterator end() const { return get(liveCount); }
        const_iterator cbegin() { return get(0); }
        const_iterator cend() { return get(liveCount); }
        const_iterator cbegin() const { return get(0); }
        const_iterator cend() const { return get(liveCount); }

        size_t size() const { return liveCount; }
        size_t capacity() const { return Max; }

        T& operator[](size_t ix) { assert(ix < liveCount); return *get(ix); }
        T const& operator[](size_t ix) const { assert(ix < liveCount); return *get(ix); }
        T& at(size_t ix) { if(ix >= liveCount) throw std::out_of_range(""); return *get(ix); }
        T const& at(size_t ix) const { if(ix >= liveCount) throw std::out_of_range(""); return *get(ix); }

    private:
        template <typename Fwd>
            void construct(Fwd b, Fwd e) {
                for(size_t i = 0; b != e; ++b, ++i) {
                    try {
                        new (get(i)) T(*b);
                    }
                    catch(...) {
                        for(size_t j = 0; j < i; ++j) {
                            get(j)->~T();
                        }
                        throw;
                    }
                }
            }

        void destroy() {
            for(size_t i = 0; i < liveCount; ++i)
                get(i)->~T();
            liveCount = 0;
        }

        size_t liveCount;

        T* get(size_t ix) { return (T*)(storage.data + sizeof(T) * ix); }
        T const* get(size_t ix) const { return (T const*)(storage.data + sizeof(T) * ix); }

        union {
            char data[sizeof(T) * Max];

            // TODO: implement alignof in clang
            double dummy0;
            double* dummy1;
            void (dummyst::*dummy2)();
        } storage;
    };
}

using avpolicy::arrayvec;


#endif // ARRAYVEC_HPP

