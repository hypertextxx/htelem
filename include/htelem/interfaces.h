#ifndef HTELEM_INTERFACES_H
#define HTELEM_INTERFACES_H

#include "htelem/attribute_list.h"
#include "htelem/element.h"
#include "htelem/static_string.h"
#include <cstdint>
namespace ht {
    template <class AttrList> struct element_interface;
    template <static_string El> struct element_interface<attribute_list<El>> {
        using attr_list_type = attribute_list<El>;
        static constexpr auto& [...x] = attr_list_type{ };
    };
}

#endif

