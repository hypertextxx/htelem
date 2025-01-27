#include <htelem/html_event.h>
#include <htelem/html.h>
#include "testing.h"

using namespace ht;

static int i = 0;
static constexpr auto basic_element = ml::div{
    ml::_onclick = [](auto&& event){ ++i; event.propagate = true; }
};
TEST(BasicEventPropagation) {
    html_event<"fake"> event{ };

    using fake_event_type = decltype(event);
    using receiver_type = std::decay_t<decltype(std::get<0>(basic_element.children))>;
    
    STATIC_EXPECT_TRUE((receiver_type::trigger_name == "onclick"));
    STATIC_EXPECT_TRUE((event_named<fake_event_type, "fake">));
    STATIC_EXPECT_TRUE((std::is_invocable_v<receiver_type, html_event<"onclick">>));
    STATIC_EXPECT_TRUE((not std::is_invocable_v<receiver_type, fake_event_type>));

    auto& result = emit_event(basic_element, event);
    EXPECT(i, std::equal_to, 0);
    
    auto result2 = emit_event(basic_element, html_event<"onclick">{ });
    EXPECT(i, std::equal_to, 1);
    EXPECT(result2.propagate, std::equal_to, true);
};

