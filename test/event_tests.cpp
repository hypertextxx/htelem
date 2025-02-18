#include <htelem/html_event.h>
#include <htelem/html.h>
#include "testing.h"

using namespace ht;
using namespace std::string_view_literals;

static int i = 0;
static constexpr auto basic_element = ml::div{
    ml::_onclick = [](auto& event){ ++i; event.propagate = true; }
};
TEST(BasicEventPropagation) {
    html_event<"fake"> event{ };

    using fake_event_type = decltype(event);
    using receiver_type = std::decay_t<decltype(std::get<0>(basic_element.children))>;
    
    STATIC_EXPECT_TRUE((receiver_type::trigger_name == "onclick"));
    STATIC_EXPECT_TRUE((event_named<fake_event_type, "fake">));
    STATIC_EXPECT_TRUE((not std::is_invocable_v<receiver_type, fake_event_type>));

    auto& result = emit_event(basic_element, event);
    EXPECT(i, std::equal_to, 0);
    EXPECT(std::addressof(result), std::equal_to, std::addressof(event));
    
    auto result2 = emit_event(basic_element, html_event<"onclick">{ });
    EXPECT(i, std::equal_to, 1);
    EXPECT(result2.propagate, std::equal_to, true);
};

TEST(DynamicMutatingElement) {
    int j = 0;
    auto dynamic_element = ml::div{ ml::_align = "left"sv, ml::_onclick = [&j](const auto& event, auto& el){ ++j; el.align = "right"sv; } };
    html_event<"onclick"> event{ };
    EXPECT(*dynamic_element.align, std::equal_to, "left"sv);

    auto& result = emit_event(dynamic_element, event);
    EXPECT(j, std::equal_to, 1);
    EXPECT(*dynamic_element.align, std::equal_to, "right"sv);
};

