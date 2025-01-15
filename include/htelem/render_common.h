#ifndef HTELEM_RENDER_COMMON_H
#define HTELEM_RENDER_COMMON_H

#include "util.h"

namespace ht {
struct render_decoration {
    const std::string_view pre;
    const std::string_view post;
};

namespace detail {
template <std::size_t Depth> consteval auto make_indent() {
    std::array<char, 1 + Depth * 4> s{};
    s.fill(' ');
    s.back() = '\0';
    return s;
}

template <std::size_t Depth> constexpr auto indent_string = static_string<Depth * 4>{make_indent<Depth>()};
template <> constexpr inline static_string indent_string<0> = static_string{""};
} // namespace detail

template <std::size_t Depth> constexpr std::string_view indent() {
    return static_cast<std::string_view>(detail::indent_string<Depth>);
}

template <class T> struct custom_renderer { };

} // namespace ht

#endif
