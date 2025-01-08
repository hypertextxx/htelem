#ifndef HTELEM_HTML_H
#define HTELEM_HTML_H

#include "element.h"
#include "html_event.h"
#include <variant>

namespace ht {

struct unimplemented { };

template <static_string EventName> struct event_handler_attr_type {
    static constexpr auto name = EventName;
};
template <static_string At, static_string EventName> struct attribute_spec<At, event_handler_attr_type<EventName>> {
    template <class U> constexpr auto operator=(U&& u) const {
        return [u](event_named<EventName> auto&& event) { return std::invoke(u, std::forward(event)); };
    }
};

namespace ml {
#include "../tools/htelem-autogen/out/attribute_spec.def"
}

#include "../tools/htelem-autogen/out/attribute_lists.def"

#include "../tools/htelem-autogen/out/interface_spec.def"

namespace ml {
#include "../tools/htelem-autogen/out/elements.def"
}
} // namespace ht

#endif // HTELEM_HTML_H
