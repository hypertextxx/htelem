#include <htelem/util.h>
#include <iterator>
#include <ranges>
#include "testing.h"

struct T1 { };
struct T2: public T1 { };
template <class T> struct is_t1: std::is_same<T, T1> { };
template <class T> struct is_int: std::is_convertible<T, int> { };

using namespace ht;
using namespace ht::detail;

TEST(FilteredIndexSeq) {
    STATIC_EXPECT_TRUE((std::is_same_v<filtered_index_sequence<is_t1, int, T1, int, int, char, T1, bool>, std::index_sequence<1, 5>>));
    STATIC_EXPECT_TRUE((std::is_same_v<filtered_index_sequence<is_int, int, bool, float, int, T1>, std::index_sequence<0, 1, 2, 3>>));
};

TEST(TupleRangeTypeTraits) {
    auto tp = std::make_tuple(5, T1{ }, T2{ }, 9);
    auto t1_range = make_tuple_range<T1>(tp);
    auto it = t1_range.begin();
    
    using t1_range_type = decltype(t1_range);
    using it_type = decltype(it);
    using it_value_type = it_type::value_type;

    STATIC_EXPECT_TRUE((std::is_same_v<it_value_type, T1>));
    STATIC_EXPECT_TRUE((std::is_same_v<std::iter_reference_t<it_type>, T1&>));
    STATIC_EXPECT_TRUE((std::random_access_iterator<it_type>));    
    STATIC_EXPECT_TRUE((std::ranges::random_access_range<t1_range_type>));
};

TEST(TupleRange) {
    auto t1 = T1{ };
    auto t2 = T2{ };
    auto tp = std::make_tuple(5, &t1, 8, &t2);
    auto r = make_tuple_range<int>(tp);
    auto it = r.begin();
    
    EXPECT(*it, std::equal_to, 5);
    ++it;
    EXPECT(*it, std::equal_to, 8);
    ++it;
    EXPECT_TRUE(it == r.end()); 

    auto r2 = make_tuple_range<T1*>(tp);
    auto it2 = r2.begin();

    STATIC_EXPECT_TRUE((std::is_same_v<std::iter_value_t<decltype(it2)>, T1*>));
    STATIC_EXPECT_TRUE((std::is_same_v<std::iter_reference_t<decltype(it2)>, T1*&>));

    EXPECT(it2[0], std::equal_to, &t1);
};

TEST(TupleRangeStringView) {
    using namespace std::literals;
    auto tp = std::make_tuple("one"sv, "two"sv, "three"sv);
    auto r = make_tuple_range<std::string_view>(tp);
    auto joined = r | std::views::join;

    using joined_type = decltype(joined);
    STATIC_EXPECT_TRUE((std::is_same_v<std::ranges::range_value_t<joined_type>, char>));

    EXPECT((std::string{ joined.begin(), joined.end() }), std::equal_to, "onetwothree");
};

