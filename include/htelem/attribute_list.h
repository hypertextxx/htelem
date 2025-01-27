#ifndef HTELEM_ATTRIBUTE_LIST_H
#define HTELEM_ATTRIBUTE_LIST_H

#include "util.h"
#include <concepts>
#include <type_traits>

namespace ht {

/**
 * \brief Template for all attribute lists.
 *
 * Each attribute list should correspond to an \ref ht::interface_spec with the same name. Specializations of
 * `attribute_list` should consist only of non-const public member variables whose types are specializations of \ref
 * ht::attribute.
 *
 * \tparam In the interface name, e.g. "HTMLDivElement"
 */
template <static_string In> struct attribute_list;

template <static_string Name, class T> constexpr T attribute_default_v;
template <static_string Name, std::default_initializable T> constexpr T attribute_default_v<Name, T> = T{};

template <class T, static_string Name> concept attribute_value_type =
        std::copyable<T> and std::equality_comparable<T> and requires {
            { attribute_default_v<Name, T> } -> std::convertible_to<T>;
        };

/**
 * \brief A named attribute which can be supplied to an element.
 *
 * The underlying value type `T` must be default-constructible and copy-constructible. Internally, when initializing an
 * element, a variable of type `T` is first default-initialized inside the `attribute_list`, then replaced by another
 * variable which is copy-initialized during \ref aspect-initialization.
 *
 * \tparam Name the attribute name, e.g. "href" for a link or "onclick" for an event handler.
 * \tparam T the underlying value type.
 */
template <static_string Name, attribute_value_type<Name> T> class attribute {
    T value;

public:
    static constexpr auto attr_name = Name;
    using attr_type = T;
    constexpr explicit attribute(const T& _value): value{_value} { }
    constexpr attribute() = default;

    constexpr T& operator()() { return value; }

    constexpr const T& operator()() const { return value; }
};

/**
 * \brief An attribute specification.
 *
 * Static global `attribute_spec`s are used to initialize attributes inside an element during \ref
 * aspect-initialization. Each `attribute_spec` has a name and a list of permissible types. For example, the HTML
 * attribute `title` is implemented as:
 * \code inline constexpr attribute_spec<"title", std::string_view> _title; \endcode
 * The expression \code _title = "Title"sv \endcode then constructs an
 * \ref ht::attribute "ht::attribute<title, std::string_view>".
 * If multiple types are permitted, then the returned `attribute` stores an \ref std::variant.
 */
template <static_string At, attribute_value_type<At>... T> struct attribute_spec {
    static constexpr auto attr_name = At;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-unconventional-assign-operator"
    template <class U> requires(std::convertible_to<U, T> or ...)
    constexpr auto operator=(U&& u) const {
        return attribute<At, detail::first_convertible_to_t<U, T...>>{std::forward<U>(u)};
    }
#pragma clang diagnostic pop
};

namespace detail {

template <class T> struct is_attribute: std::false_type { };
template <static_string Name, class T> struct is_attribute<attribute<Name, T>>: std::true_type { };

template <class T> struct is_attribute_ptr: std::false_type { };
template <static_string AttrName, class AttrType, static_string In>
struct is_attribute_ptr<attribute<AttrName, AttrType> attribute_list<In>::*>: std::true_type { };

template <class T> struct is_not_attribute: std::bool_constant<not is_attribute<T>::value> { };

template <class T> struct get_attribute_name: std::false_type { };
template <static_string AttrName, class T> struct get_attribute_name<attribute<AttrName, T>>: std::true_type {
    static constexpr auto name = AttrName;
};

/// Given an attribute name `AttrName`, searches among the attribute member pointers `AttrPtr` until a matching name is
/// found, then extracts the name of the interface in which it lives.
template <static_string AttrName, class... AttrPtr> struct find_attr_holder;
template <static_string AttrName> struct find_attr_holder<AttrName>: std::false_type { };
template <static_string AttrName, class A, class... Tail> struct find_attr_holder<AttrName, A, Tail...>
    : find_attr_holder<AttrName, Tail...> { };

template <static_string AttrName, static_string In, class AttrType, class... Tail>
struct find_attr_holder<AttrName, attribute<AttrName, AttrType> attribute_list<In>::*, Tail...>: std::true_type {
    static constexpr auto interface_name = In;
    static constexpr auto attr_name = AttrName;
    using attr_value_type = AttrType;
};

template <class Attr, class PtrTuple> struct find_attr_holder_in_tuple;
template <static_string AttrName, class AttrType, class... AttrPtr>
struct find_attr_holder_in_tuple<attribute<AttrName, AttrType>, std::tuple<AttrPtr...>>
    : find_attr_holder<AttrName, AttrPtr...> { };

/// Given an attribute member pointer, extract the name of the interface in which it lives.
template <auto A> struct interface_name_from_ptr { };
template <static_string In, static_string AttrName, class AttrType,
        attribute<AttrName, AttrType> attribute_list<In>::* Ptr>
struct interface_name_from_ptr<Ptr> {
    static constexpr auto value = In;
};

/// Determines if a pack of attribute member pointers `AttrPtrs` contains any attribute with the name `AttrName`.
template <static_string AttrName, class... AttrPtrs> constexpr bool attributes_contain_v =
        find_attr_holder<AttrName, AttrPtrs...>::value;

/**
 * \defgroup aspect-initialization aspect-initialization
 * \brief In order to more closely mirror the syntax of HTML, elements have their own initialization semantics. Let `el`
 * be an element type with interface name "ElElement" and parent interface name "ParentElement". A list-initialization
 * statement for a variable of type `el` has the form: \code el{ A1, ... , An } \endcode -- we will call each of the
 * `A`s an _aspect_. An _aspect_ `A` may be an _attribute_, i.e. `A` has type ht::attribute<AttrName, T>. Otherwise, `A`
 * is a _child_.
 *
 * The following occurs during the initialization of the ht::element base of `el`:
 * - An empty std::tuple, the children-tuple, is constructed.
 * At each aspect `A`:
 * - If `A` is an attribute of type ht::attribute<"attr", T>:
 *     - The corresponding pointer `ptr` to a member of the ht::attribute_list<"ElElement"> or the
 * ht::attribute_list<"ParentElement"> base of `el` is located.
 *         - If no ht::attribute_list base of `el` contains a member of type ht::attriubte<"attr", T>, the program is
 * ill-formed.
 *     - The base class member referred to by `ptr` is assigned to an ht::attribute whose underlying value is
 * copy-initialized from `A`.
 * - If `A` is a child of type `V`, suppose the children-tuple has type std::tuple<C...>. The children-tuple is replaced
 * by an std::tuple<C..., V&&>, where the final element is forwarded from `A`.
 */

template <static_string El, class... AttrPtrs, class... Children>
constexpr auto initialize_aspect(attribute_list<El>& t, std::tuple<AttrPtrs...> ptrs,
        std::tuple<Children...> children) {
    return std::tuple<>{};
}

template <class AttrName, class... AttrPtrs, class V>
constexpr auto take_attr(auto& t, const std::tuple<AttrPtrs...>& ptrs, V&& value) {
    if constexpr (AttrName::value) {
        static_assert(attributes_contain_v<AttrName::name, AttrPtrs...>,
                "attribute does not apply to the specified element");
        constexpr auto attr_holder = find_attr_holder<AttrName::name, AttrPtrs...>::interface_name;

        using attr_ptr_type = std::remove_reference_t<V> attribute_list<attr_holder>::*;
        t.*(std::get<attr_ptr_type>(ptrs)) = value;
    }
};

template <class AttrName, class... Children, class V>
constexpr auto take_child(auto& t, std::tuple<Children...>&& children, V&& value) {
    if constexpr (!AttrName::value) {
        return std::apply([&value](const Children&... c) { return std::make_tuple(c..., std::forward<V>(value)); },
                std::move(children));
        // something is wrong with libcxx or with me idk
        // return std::tuple_cat(std::move(children), std::make_tuple(std::forward<V>(value)));
    } else {
        return children;
    }
};

/**
 * \ingroup aspect-initialization
 * \brief Perform aspect-initialization for a given aspect inside the initialization of an element.
 *
 * \tparam In the element's interface name
 * \tparam AttrPtrs the pack of attribute member pointers combined from the element's interface and its parents
 * \tparam V the type of the aspect currently being initialized
 * \tparam R the pack of aspects yet to be initialized
 * \tparam Children the pack of children that have already been initialized
 */
template <static_string In, class... AttrPtrs, class V, class... R, class... Children>
constexpr auto initialize_aspect(attribute_list<In>& t, const std::tuple<AttrPtrs...>& ptrs,
        std::tuple<Children...>&& children, V&& value, R&&... r) {
    using attr_name = get_attribute_name<std::remove_cvref_t<V>>;

    take_attr<attr_name>(t, ptrs, std::forward<V>(value));

    if constexpr (sizeof...(R) == 0) {
        return take_child<attr_name>(t, std::move(children), std::forward<V>(value));
    } else {
        return initialize_aspect(t, ptrs,
                std::move(take_child<attr_name>(t, std::move(children), std::forward<V>(value))),
                std::forward<R>(r)...);
    }
}
} // namespace detail
} // namespace ht

#endif // HTELEM_ATTRIBUTE_LIST_H
