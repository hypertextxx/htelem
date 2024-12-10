#include "testing.h"
#include "simple_elements.h"

using namespace ht;
using namespace ht::ml;

TEST(AttributeSpecsMakeAttributes) {
    EXPECT_TRUE((std::is_same_v<decltype(_aaa = "test"), attribute<"aaa", std::string_view>>));
    EXPECT_TRUE((std::is_same_v<decltype(_bbb = "test"), attribute<"bbb", std::string_view>>));
    EXPECT_TRUE((std::is_same_v<decltype(_bbb = y{ 'A' }), attribute<"bbb", y>>));
};

TEST(SimpleElementsWithAttributes) {
    static constexpr auto x = parent { _aaa = "parent argument" };
    static constexpr auto z = child { _aaa = "test", _bbb = y{ 'c' } };
    static constexpr auto w = child2 { _bbb = "other text" };

    EXPECT(x.aaa(), std::equal_to, "parent argument");
    EXPECT(z.aaa(), std::equal_to, "test");
    EXPECT(z.bbb().c, std::equal_to, 'c');
    EXPECT(w.bbb(), std::equal_to, "other text");
};

TEST(SimpleElementsWithChildren) { 
    using parent_type = decltype(parent { _aaa = "parent argument", child { _aaa = "child argument" } }); 
    using parent_set_attr_type = parent_type::set_attrs_tuple;

    static constexpr auto x = parent { _aaa = "parent argument", child { _aaa = "child argument" } };
    static constexpr auto z = child { _aaa = "test", y{ 'r' } };

    EXPECT(std::get<0>(x.children()).aaa(), std::equal_to, "child argument");
    EXPECT(std::tuple_size_v<decltype(z)::set_attrs_tuple>, std::equal_to, 1);
    EXPECT(std::get<0>(z.children()).c, std::equal_to, 'r');
};

