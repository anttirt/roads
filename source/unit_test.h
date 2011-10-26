#ifndef DSR_UNIT_TEST_H
#define DSR_UNIT_TEST_H

#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

namespace roads
{
    struct unit_test_failure
        : std::exception
    {
        unit_test_failure(std::string const& msg)
            : m_what(msg) {}

        ~unit_test_failure() throw() {}
        virtual char const* what() const throw() { return m_what.c_str(); }

    private:
        std::string m_what;
    };

    struct unit_test_base
    {
        virtual std::string const& name() const = 0;
        virtual void run() = 0;
        virtual ~unit_test_base() {}
    };

    struct unit_test_suite
    {
        unit_test_suite() {}

        void add_test(std::auto_ptr<unit_test_base> test)
        {
            m_tests.push_back(test.release());
        }

        void run_tests();
        ~unit_test_suite();

    private:
        unit_test_suite(unit_test_suite const&);
        unit_test_suite& operator=(unit_test_suite const&);

        struct deleter { template <typename T> T* operator()(T* p) { delete p; return 0; } };
        std::vector<unit_test_base*> m_tests;
    };

    template <typename T>
    void create_tests(unit_test_suite&);

    void throw_on_fail(bool success, char const* msg, ...) __attribute__((format(printf, 2, 3)));

#define UASSERT(x, ...)                    \
    do {                                   \
        throw_on_fail(!!(x), __VA_ARGS__); \
    } while(0)

#define UASSERT_EQUAL(x, y)                      \
    do {                                         \
        std::string x_str =                      \
            boost::lexical_cast<std::string>(x); \
        std::string y_str =                      \
            boost::lexical_cast<std::string>(y); \
        UASSERT((x) == (y),                      \
            #x " != " #y "[%s should be %s]",    \
            x_str.c_str(),                       \
            y_str.c_str());                      \
    } while(0)
#define UASSERT_NOT_EQUAL(x, y) \
    do {                                         \
        std::string x_str =                      \
            boost::lexical_cast<std::string>(x); \
        std::string y_str =                      \
            boost::lexical_cast<std::string>(y); \
        UASSERT((x) != (y),                      \
            #x " == " #y "[%s]",    \
            x_str.c_str(),                       \
            y_str.c_str());                      \
    } while(0)
#define UNIT_TEST(test_name, ...)                    \
    std::string const test_name##_name = #test_name; \
    struct test_name : unit_test_base {              \
        std::string const& name() const              \
            { return test_name##_name; }             \
        void run()                                   \
        __VA_ARGS__                                  \
    }

}

#endif // DSR_UNIT_TEST_H
