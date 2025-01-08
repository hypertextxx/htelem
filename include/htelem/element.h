#ifndef HTELEM_ELEMENT_H
#define HTELEM_ELEMENT_H

#include "attribute_list.h"
#include "interface_spec.h"
#include "util.h"
#include <tuple>
#include <utility>

namespace ht {
template <static_string Tag, class Spec, class Cs> struct element;

template <static_string Tag, static_string In, class Parent, auto... Attrs, class... Aspects>
struct element<Tag, interface_spec<In, Parent, Attrs...>, std::tuple<Aspects...>>: public attribute_list<In> {
    using interface = interface_spec<In, Parent, Attrs...>;
    using set_attrs_tuple =
            extract_attr_ptrs<typename detail::filter_types<detail::is_attribute, Aspects...>::type, interface>::type;
    using children_tuple = detail::filter_types<detail::is_descendant, Aspects...>::type;
    static constexpr auto tag_name = Tag;

    element_aspects<set_attrs_tuple, children_tuple> aspects;

    constexpr element(const element& other) = default;
    constexpr element(element&& other) = default;

    template <class... T> requires(sizeof...(Aspects) > 0)
    constexpr explicit element(T&&... t)
        : attribute_list<In>{}, aspects{(interface::make(*this, detail::cstr_to_sv(std::forward<T>(t))...))} { }

    template <class... T> requires(sizeof...(Aspects) == 0)
    constexpr explicit element(T&&... t): attribute_list<In>{} {
        interface::make(*this, std::forward<T>(t)...);
    }

    constexpr auto& set_attrs() { return aspects.set_attrs; }
    constexpr auto& children() { return aspects.children; }
    constexpr const auto& set_attrs() const { return aspects.set_attrs; }
    constexpr const auto& children() const { return aspects.children; }
};

template <class T, class = void> constexpr bool is_element_type_v = false;
template <class T> constexpr bool is_element_type_v<T, std::void_t<decltype(T::tag_name)>> = true;
} // namespace ht

#endif // HTELEM_ELEMENT_H
