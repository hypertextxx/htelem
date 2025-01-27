#ifndef HTELEM_ELEMENT_H
#define HTELEM_ELEMENT_H

#include "attribute_list.h"
#include "interface_spec.h"
#include "util.h"
#include <tuple>
#include <utility>

namespace ht {

/// See the \link element_primary_template primary template \endlink.
template <static_string Tag, class Spec, class... Aspects> struct element;

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
struct element<Tag, interface_spec<In, Parent, Attrs...>, Aspects...>: public attribute_list<In> {
    using interface = interface_spec<In, Parent, Attrs...>;
    using children_tuple = detail::filter_types<detail::is_not_attribute, Aspects...>::type;
    using set_attrs_tuple = detail::filter_types<detail::is_attribute, Aspects...>::type;
    static constexpr auto tag_name = Tag;

    children_tuple children;

    constexpr element(const element& other) = default;
    constexpr element(element&& other) = default;

    /// Performs \ref aspect-initialization for the element.
    constexpr explicit element(Aspects&&... t) requires(std::tuple_size_v<children_tuple> > 0)
        : attribute_list<In>{}, children{interface::make(*this, std::forward<Aspects>(t)...)} { }

    constexpr explicit element(Aspects&&... t) requires(std::tuple_size_v<children_tuple> == 0)
        : attribute_list<In>{} {
        interface::make(*this, std::forward<Aspects>(t)...);
    }
};

/// Satisfied when an lvalue reference to a specialization of \ref ht::element can bind to `T`, and in particular when
/// `T` is derived from such a specialization.
template <class T> concept element_type =
        requires(T t) { []<static_string Tag, class Spec, class... Cs>(element<Tag, Spec, Cs...>&) { }(t); };

} // namespace ht

#endif // HTELEM_ELEMENT_H
