#ifndef HTELEM_HTML_H
#define HTELEM_HTML_H

#include "element.h"
#include "html_event.h"
#include <variant>

namespace ht {

struct unimplemented {
    constexpr bool operator==(const unimplemented&) const { return true; }
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
