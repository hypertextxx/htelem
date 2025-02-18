#include "testing.h"
#include "simple_elements.h"
#include <type_traits>

using namespace ht;
using namespace ht::ml;
using namespace std::string_view_literals;

TEST(AttributeSpecsMakeAttributes) {
    STATIC_EXPECT_TRUE((std::is_same_v<decltype(_aaa = "test"), attribute<"aaa", std::string_view>>));
    STATIC_EXPECT_TRUE((std::is_same_v<decltype(_bbb = "test"), attribute<"bbb", std::string_view>>));
    STATIC_EXPECT_TRUE((std::is_same_v<decltype(_bbb = y{ 'A' }), attribute<"bbb", y>>));
};

TEST(SimpleElementsWithAttributes) {
    static constexpr auto x = parent { _aaa = "parent argument" };
    static constexpr auto z = child { _aaa = "test", _bbb = y{ 'c' } };
    static constexpr auto w = child2 { _bbb = "other text" };
    STATIC_EXPECT(*x.aaa, std::equal_to, "parent argument"sv);
    STATIC_EXPECT(*z.aaa, std::equal_to, "test"sv);
    STATIC_EXPECT(z.bbb->c, std::equal_to, 'c');
    STATIC_EXPECT(*w.bbb, std::equal_to, "other text"sv);
};

TEST(SimpleElementsWithChildren) { 
    using parent_type = decltype(parent { _aaa = "parent argument", child { _aaa = "child argument" } }); 
    using child_type = decltype(child{ _aaa = "child argument" });
    using parent_set_attr_type = parent_type::set_attrs_tuple;
    static_assert(std::is_nothrow_copy_constructible_v<child_type>);

    static constexpr auto x = parent { _aaa = "parent argument", child { _aaa = "child argument" } };
    static constexpr auto z = child { _aaa = "test", y{ 'r' } };

    STATIC_EXPECT(*std::get<0>(x.children).aaa, std::equal_to, "child argument");
    STATIC_EXPECT(std::tuple_size_v<decltype(z)::set_attrs_tuple>, std::equal_to, 1);
    STATIC_EXPECT(std::get<0>(z.children).c, std::equal_to, 'r');
};

