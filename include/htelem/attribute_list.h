#ifndef HTELEM_ATTRIBUTE_LIST_H
#define HTELEM_ATTRIBUTE_LIST_H

#include "util.h"
#include <type_traits>

namespace ht {
template <static_string El> struct attribute_list;

template <static_string Name, class T> class attribute {
    T value;

public:
    static constexpr auto attr_name = Name;
    using attr_type = T;
    bool initialized = false;
    constexpr explicit attribute(const T& _value): value{ _value }, initialized{ true } { }
    constexpr attribute() = default;

    constexpr T& operator()() { return value; }

    constexpr const T& operator()() const { return value; }
};

template <static_string At, class... T> struct attribute_spec {
    static constexpr auto attr_name = At;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-unconventional-assign-operator"
    template <class U>
        requires(std::convertible_to<U, T> or ...)
    constexpr auto operator=(U&& u) const {
        return attribute<At, detail::first_convertible_to_t<U, T...>>{ std::forward<U>(u) };
    }
#pragma clang diagnostic pop
};

template <class AttrsTuple, class ChildrenTuple> struct element_aspects {
    AttrsTuple set_attrs;
    ChildrenTuple children;

    constexpr element_aspects(element_aspects& other) = default;
    constexpr element_aspects(element_aspects&& other) = default;

    constexpr element_aspects() { }
    constexpr element_aspects(AttrsTuple&& _set_attrs, ChildrenTuple&& _children)
        : set_attrs{ std::move(_set_attrs) }, children{ std::move(_children) } { }
};

namespace detail {

    template <class T> struct is_attribute: std::false_type { };
    template <static_string Name, class T> struct is_attribute<attribute<Name, T>>: std::true_type { };

    template <class T> struct is_attribute_ptr: std::false_type { };
    template <static_string AttrName, class AttrType, static_string In>
    struct is_attribute_ptr<attribute<AttrName, AttrType> attribute_list<In>::*>: std::true_type { };

    template <class T> struct is_descendant: std::bool_constant<not is_attribute<T>::value> { };

    template <class T> struct get_attribute_name: std::false_type { };
    template <static_string AttrName, class T> struct get_attribute_name<attribute<AttrName, T>>: std::true_type {
        static constexpr auto name = AttrName;
    };

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

    template <auto A> struct interface_name_from_ptr { };
    template <static_string In, static_string AttrName, class AttrType,
              attribute<AttrName, AttrType> attribute_list<In>::*Ptr>
    struct interface_name_from_ptr<Ptr> {
        static constexpr auto value = In;
    };

    template <class T> constexpr auto cstr_to_sv(T&& t) { return std::forward<T>(t); }
    template <std::size_t N> constexpr auto cstr_to_sv(const char (&c)[N]) { return std::string_view{ c, N - 1 }; }

    template <class T> struct child_type_map: std::type_identity<T> { };
    template <std::size_t N> struct child_type_map<const char (&)[N]>: std::type_identity<std::string_view> { };
    template <class T> using child_mapped_type_t = child_type_map<T>::type;

    template <class... T> using child_types = std::tuple<child_mapped_type_t<T>...>;

    template <static_string AttrName, class... AttrPtrs> constexpr bool attributes_contain_v =
            find_attr_holder<AttrName, AttrPtrs...>::value;

    template <static_string El, class... AttrPtrs, class... SetAttrs, class... Children>
    constexpr auto initialize_attribute(attribute_list<El>& t, std::tuple<AttrPtrs...> ptrs, std::tuple<SetAttrs...>,
                                        std::tuple<Children...> children) {
        return element_aspects<std::tuple<>, std::tuple<>>{};
    }

    template <static_string El, class... AttrPtrs, class V, class... R, class... SetAttrs, class... Children>
    constexpr auto initialize_attribute(attribute_list<El>& t, std::tuple<AttrPtrs...> ptrs,
                                        std::tuple<SetAttrs...> set_attrs, std::tuple<Children...> children, V&& value,
                                        R&&... r) {
        using attr_name = get_attribute_name<std::remove_cvref_t<V>>;

        static constexpr auto take_attr = [](auto& t, auto& ptrs, auto&& set_attrs, auto&& value) {
            if constexpr (attr_name::value) {
                static_assert(attributes_contain_v<attr_name::name, AttrPtrs...>,
                              "attribute does not apply to the specified element");
                constexpr auto attr_holder = find_attr_holder<attr_name::name, AttrPtrs...>::interface_name;

                using attr_ptr_type = std::remove_reference_t<V> attribute_list<attr_holder>::*;
                t.*(std::get<attr_ptr_type>(ptrs)) = value;
                return std::tuple_cat(std::move(set_attrs), std::make_tuple(std::get<attr_ptr_type>(ptrs)));
            } else {
                return set_attrs;
            }
        };

        static constexpr auto take_child = [](auto& t, auto&& children, auto&& value) {
            if constexpr (!attr_name::value) {
                return std::tuple_cat(std::move(children), std::make_tuple(std::forward<V>(value)));
            } else {
                return children;
            }
        };

        if constexpr (sizeof...(R) == 0) {
            return element_aspects{ std::move(take_attr(t, ptrs, set_attrs, value)),
                                    std::move(take_child(t, children, value)) };
        } else {
            return initialize_attribute(t, std::move(ptrs), take_attr(t, ptrs, set_attrs, value),
                                        take_child(t, children, value), std::forward<R>(r)...);
        }
    }
} // namespace detail
} // namespace ht

#endif // HTELEM_ATTRIBUTE_LIST_H
