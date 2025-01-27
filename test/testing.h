#ifndef HTELEM_TESTING_H
#define HTELEM_TESTING_H

#include <type_traits>
#include <functional>
#include <htelem/util.h>
#include <print>

namespace testing {
    using ht::static_string;
    constexpr auto preformat_test_value(const auto& t) { return t; }
    inline auto preformat_test_value(auto* t) { return std::bit_cast<uintptr_t>(t); }

    struct controller {
        const std::string_view name;
        std::function<void(controller&)> func;
        std::size_t failures = 0;

        template <std::invocable<controller&> Func> controller(const std::string_view _name, Func&& _func): name { _name }, func { std::forward<Func>(_func) } { }

        template <template<class> class Checker = std::equal_to, class L, class R> constexpr bool expect(const L& actual, const R& expected, const std::string&& expr) {
            static_assert(std::is_convertible_v<R, L>);
            using namespace std;
            if (!Checker<L>{ }(actual, expected)) {
                std::print(stderr, "  failed: {}\n    expected: {}\n   actual: {}\n", expr, preformat_test_value(expected), preformat_test_value(actual));
                failures++;
                return false;
            }
#ifdef HTELEM_VERBOSE_TESTS
            else {
                std::print("  passed: {}\n", expr);
                if (!std::is_same_v<std::decay_t<L>, bool>) {
                    std::print("    value: {}\n", preformat_test_value(actual));
                }
            }
#endif
            return true;
        }

        inline std::size_t operator()() {
            std::invoke(func, *this);
            return failures;
        }
    };
    extern std::vector<controller> all_tests;

    template <static_string Name> struct test_impl {
        static constexpr auto _name = Name;
        template <std::invocable<controller&> Func> explicit(false) inline test_impl(Func&& _func) {
            all_tests.emplace_back(testing::controller{ Name, std::forward<Func>(_func) });
        }
    };
}

// i feel like i need to explain this insanity real quick - since test units are registered in the all_tests vector
// during static initialization, the order in which they are added is undefined. practically they will usually
// go in the order they are defined, but in case we need to ensure one test runs after another, it's enough to
// access the dependency's test_impl, but it's not enough to just reference the type. this, shockingly, seemed
// like the cleanest way to reference the name and discard it while still allowing for the clean lambda
// syntax the TEST macro uses.
template <std::size_t N, std::invocable<testing::controller&> Func> constexpr auto operator|(const testing::static_string<N> after, Func&& _func) {
    (void) after;
    return std::forward<Func>(_func);
}

/**
 * Defines a test unit. May specify up to one optional dependency which will be forced to run before this one.
 */
#define TEST(name, ...) extern const testing::test_impl<#name> test_ ## name = __VA_OPT__((test_ ## __VA_ARGS__)._name |) [](testing::controller& _controller) -> void
#define EXPECT_TRUE(cond) (_controller.expect(static_cast<bool>((cond)), true, #cond))
#define STATIC_EXPECT_TRUE(cond) static_assert(cond); EXPECT_TRUE(cond)

/**
 * Expects the given comparison to succeed. `actual` and `expected` must be references to types satisfying std::is_convertible_v
 * and `comparison` must be a type name such that `comparison<T>{ }(actual, expected)` is a well-formed expression that returns
 * `bool`, such as `std::equal_to`.
 */
#define EXPECT(actual, comparison, expected) _controller.expect<comparison>(actual, expected, #actual " " #comparison " " #expected)
#define STATIC_EXPECT(actual, comparison, expected) static_assert(comparison{ }(actual, expected)); EXPECT(actual, comparison, expected)

#define REQUIRE_TRUE(cond) do { if (!EXPECT_TRUE(cond)) return; } while(0)
#define REQUIRE(actual, comparison, expected) do { if (!EXPECT(actual, comparison, expected)){ return; } } while(0)

#endif //HTELEM_TESTING_H
