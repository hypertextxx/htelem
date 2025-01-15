#ifndef HTELEM_ELEMENT_H
#define HTELEM_ELEMENT_H

#include "attribute_list.h"
#include "interface_spec.h"
#include "util.h"
#include <tuple>
#include <utility>

namespace ht {

/// See the \link element_primary_template primary template \endlink.
template <static_string Tag, class Spec, class Cs> struct element;

/**
 * \anchor element_primary_template
 * \brief Base type for all elements.
 *
 * \tparam Tag the element tag, e.g. "div"
 * \tparam In the interface name, i.e. "HTMLDivElement"
 * \tparam Parent an \ref std::tuple of \ref ht::interface_spec specializations
 * \tparam Attrs a pack of pointers to members of \ref ht::attribute_list "ht::attribute_list<In>"
 * \tparam Aspects a pack of types for both attributes and children of the element
 */
template <static_string Tag, static_string In, class Parent, auto... Attrs, class... Aspects>
struct element<Tag, interface_spec<In, Parent, Attrs...>, std::tuple<Aspects...>>: public attribute_list<In> {
    using interface = interface_spec<In, Parent, Attrs...>;
    using set_attrs_tuple =
            extract_attr_ptrs<typename detail::filter_types<detail::is_attribute, Aspects...>::type, interface>::type;
    using children_tuple = detail::filter_types<detail::is_not_attribute, Aspects...>::type;
    static constexpr auto tag_name = Tag;

    element_aspects<set_attrs_tuple, children_tuple> aspects;

    constexpr element(const element& other) = default;
    constexpr element(element&& other) = default;

    /// Performs \ref aspect-initialization for the element.
    template <class... T> requires(sizeof...(Aspects) > 0)
    constexpr explicit element(T&&... t)
        : attribute_list<In>{}, aspects{(interface::make(*this, detail::cstr_to_sv(std::forward<T>(t))...))} { }

    template <class... T> requires(sizeof...(Aspects) == 0)
    constexpr explicit element(T&&... t): attribute_list<In>{} {
        interface::make(*this, std::forward<T>(t)...);
    }

    /// Obtains an \ref std::tuple of attribute member pointers that have been explicitly set by the user.
    constexpr auto& set_attrs() { return aspects.set_attrs; }

    /// Obtains an \ref std::tuple of children of this element.
    constexpr auto& children() { return aspects.children; }
    constexpr const auto& set_attrs() const { return aspects.set_attrs; }
    constexpr const auto& children() const { return aspects.children; }
};

/// Satisfied when an lvalue reference to a specialization of \ref ht::element can bind to `T`, and in particular when
/// `T` is derived from such a specialization.
template <class T> concept element_type =
        requires(T t) { []<static_string Tag, class Spec, class Cs>(element<Tag, Spec, Cs>&) { }(t); };

} // namespace ht

#endif // HTELEM_ELEMENT_H
