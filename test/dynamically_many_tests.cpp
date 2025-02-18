#include <htelem/dynamically_many.h>
#include "htelem/interface_spec.h"
#include "testing.h"

using namespace ht;
using namespace std::literals;

inline constexpr attribute_spec<"i", std::string_view> _i;

template <> struct attribute_list<"Repeatable"> {
    attribute<"i", std::string_view> i;
};
using Repeatable = interface_spec<"Repeatable", std::tuple<>, &attribute_list<"Repeatable">::i>;

template <class ...Aspects> struct repeatable: public element<"repeatable", Repeatable, Aspects...> {
    using element<"repeatable", Repeatable, Aspects...>::element;
};
template <class ...Aspects> repeatable(Aspects&&...) -> repeatable<Aspects...>;

TEST(DynamicallyManyTest) {
    static constexpr auto e1 = dynamically_many { repeatable { _i = "example"sv }, for_example };
    STATIC_EXPECT(e1.children.size(), std::equal_to, 0);

    auto e2 = dynamically_many { repeatable { _i = "first"sv },
                                 repeatable { _i = "second"sv } };
    EXPECT(e2.children.size(), std::equal_to, 2);
    
    e2.children.emplace_back(repeatable{ _i = "third"sv } );
    EXPECT(*e2.children.back().i, std::equal_to, "third"sv);
};

