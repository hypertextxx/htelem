#ifndef HTELEM_DYNAMICALLY_MANY_H
#define HTELEM_DYNAMICALLY_MANY_H

#include "element.h"
#include <type_traits>
#include <vector>

namespace ht {
namespace detail {
struct for_example_tag { };
} // namespace detail

static constexpr detail::for_example_tag for_example{};

template <class T> struct dynamically_many {
    std::vector<T> children;

    template <class... U> constexpr dynamically_many(U&&... u): children{std::forward<T>(u)...} { }
    constexpr dynamically_many(T, detail::for_example_tag) { }
};
template <class... U> dynamically_many(U&&...) -> dynamically_many<std::common_type_t<U...>>;
} // namespace ht

#endif

