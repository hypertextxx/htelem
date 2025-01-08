#ifndef HTELEM_SIMPLE_ELEMENTS_H
#define HTELEM_SIMPLE_ELEMENTS_H

#include <htelem/element.h>
#include <type_traits>

struct y {
    char c;
};

namespace ht {
    namespace ml {
        inline constexpr attribute_spec<"aaa", std::string_view> _aaa;
        inline constexpr attribute_spec<"bbb", y, std::string_view> _bbb;
        inline constexpr attribute_spec<"ccc", double> _ccc;
    }

    template <> struct attribute_list<"Parent"> {
        attribute<"aaa", std::string_view> aaa;
    };

    template <> struct attribute_list<"Child">: attribute_list<"Parent"> {
        attribute<"bbb", y> bbb;
    };

    template <> struct attribute_list<"Child2">: attribute_list<"Parent"> {
        attribute<"bbb", std::string_view> bbb;
        attribute<"ccc", double> ccc;
    };

    namespace interface {
        using Parent = interface_spec<"Parent", std::tuple<>, &attribute_list<"Parent">::aaa>;
        using Child = interface_spec<"Child", std::tuple<Parent>, &attribute_list<"Child">::bbb>;
        using Child2 = interface_spec<"Child2", std::tuple<Parent>, &attribute_list<"Child2">::bbb, &attribute_list<"Child2">::ccc>;
    }

    namespace ml {
        template <class Cs> struct parent: public element<"parent", ::ht::interface::Parent, Cs> {
            using element<"parent", ::ht::interface::Parent , Cs>::element;
        };
        template <class ...T> parent(T&& ...t) -> parent<typename ::ht::detail::child_types<T...>>;

        template <class Cs> struct child: public element<"child", ::ht::interface::Child, Cs> {
            using element<"child", ::ht::interface::Child , Cs>::element;
        };
        template <class ...T> child(T&& ...t) -> child<typename ::ht::detail::child_types<T...>>;

        template <class Cs> struct child2: public element<"child", ::ht::interface::Child2, Cs> {
            using element<"child", ::ht::interface::Child2 , Cs>::element;
        };
        template <class ...T> child2(T&& ...t) -> child2<typename ::ht::detail::child_types<T...>>;
    }
}

#endif //HTELEM_SIMPLE_ELEMENTS_H
