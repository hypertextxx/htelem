#ifndef HTELEM_INTERFACE_SPEC_H
#define HTELEM_INTERFACE_SPEC_H

#include "attribute_list.h"
#include "util.h"

namespace ht {

/**
 * \brief An element's interface specification.
 *
 * An interface specification consists of an interface name, zero or more parent interface specifications, and zero or
 * more attributes. Every element must have an interface specification, but multiple elements may use the same interface
 * specification -- for example, simple text elements like \<b\> and \<i\> use the `HTMLElement` interface. An element
 * which accepts user-defined attributes should provide an alias to a new specialization of `interface_spec`.
 *
 * \tparam In the interface name, e.g. "HTMLDivElement"
 * \tparam Parent an \ref std::tuple of parent `interface_spec`s
 * \tparam A a pack of pointers to members of \ref ht::attribute_list "ht::attribute_list<In>"
 */
template <static_string In, class Parent, auto... A> struct interface_spec;

template <class Spec> struct get_interface_attrs;
template <static_string In, class Parent, auto... A> struct get_interface_attrs<interface_spec<In, Parent, A...>> {
    static constexpr auto value = std::make_tuple(A...);
};

/// Obtains a tuple containing the attribute member pointers for an interface and all of its parent interfaces
/// recursively.
template <class Spec> struct combined_interface_attrs;
template <static_string In, class... Parents, auto... Attrs>
struct combined_interface_attrs<interface_spec<In, std::tuple<Parents...>, Attrs...>> {
    static constexpr auto value =
            std::tuple_cat(get_interface_attrs<interface_spec<In, std::tuple<Parents...>, Attrs...>>::value,
                    get_interface_attrs<Parents>::value...);
};
template <class Spec> constexpr auto combined_interface_attrs_v = combined_interface_attrs<Spec>::value;

template <class Tuple, class Interface> struct extract_attr_ptrs;
template <class... AttrAspects, class Interface> struct extract_attr_ptrs<std::tuple<AttrAspects...>, Interface> {
    static constexpr auto combined = combined_interface_attrs_v<Interface>;
    using type = std::tuple<AttrAspects attribute_list<
            detail::find_attr_holder_in_tuple<AttrAspects, std::decay_t<decltype(combined)>>::interface_name>::*...>;
};

template <static_string In, class... AttrType, class... Parents, AttrType attribute_list<In>::*... A>
struct interface_spec<In, std::tuple<Parents...>, A...> {
    static constexpr auto name = In;
    static constexpr auto ptrs = get_interface_attrs<interface_spec<In, std::tuple<Parents...>, A...>>::value;
    static constexpr auto inherited_ptrs = std::tuple_cat(get_interface_attrs<Parents>::value...);
    using parent_types = std::tuple<Parents...>;

    template <class... T> static constexpr auto make(attribute_list<In>& list, T&&... t) {
        return detail::initialize_aspect(list, std::tuple_cat(ptrs, inherited_ptrs), std::make_tuple(),
                std::make_tuple(), std::forward<T>(t)...);
    }
};
} // namespace ht

#endif // HTELEM_INTERFACE_SPEC_H
