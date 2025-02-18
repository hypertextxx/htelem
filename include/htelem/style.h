#ifndef HTELEM_STYLE_H
#define HTELEM_STYLE_H

#include "htelem/_utilities/static_string.h"

namespace ht {
    template <static_string In> struct style {
        constexpr auto operator<=>(const style&) const = default;
    };
}

#endif

