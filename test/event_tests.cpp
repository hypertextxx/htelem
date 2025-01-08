#include <htelem/html_event.h>
#include <htelem/html.h>
#include "testing.h"

using namespace ht;

static int i = 0;
static constexpr auto basic_element = ml::div{
    ml::_onclick = [](event_named<"fake"> auto&& event){ ++i; event.propagate = true; }
};
TEST(BasicEventPropagation) {
    html_event<"fake"> event{ };
    auto& result = propagate_event(basic_element, event);
    EXPECT(i, std::equal_to, 1);
    
    auto result2 = propagate_event(basic_element, html_event<"fake">{ });
    EXPECT(i, std::equal_to, 2);
    EXPECT(result2.propagate, std::equal_to, true);
};

