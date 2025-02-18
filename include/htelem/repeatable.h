#ifndef HTELEM_DYNAMICALLY_MANY_H
#define HTELEM_DYNAMICALLY_MANY_H

#include "element.h"
#include <memory>
#include <vector>

namespace ht {
template <class T> struct for_example {
    using type = T;

    constexpr explicit for_example(T&& t) { }
};

template <class T, std::size_t N> class dynamically_many {
    std::vector<T> children;
    std::array<T, N> static_children;

    template <class U> constexpr void add_child(U&& u) { children.push_back(std::forward(u)); }
    template <class U> constexpr void add_child(for_example<U>) { }

public:
    template <class... U> constexpr dynamically_many(T&& t, U&&... u): static_children{std::move(t), std::move(u)...} {
        if not consteval {
            std::uninitialized_move_n(static_children.begin(), N, children.begin());
        }
    }

    constexpr dynamically_many(for_example<T>) { }
};

} // namespace ht

#endif

