#ifndef HTELEM_SIMPLE_ELEMENTS_H
#define HTELEM_SIMPLE_ELEMENTS_H

#include <htelem/element.h>

struct y {
    char c;

    constexpr auto operator<=>(const y& other) const = default;
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
        template <class ...Aspects> struct parent: public element<"parent", ::ht::interface::Parent, Aspects...> {
            using element<"parent", ::ht::interface::Parent , Aspects...>::element;
        };
        template <class ...Aspects> parent(Aspects&&...) -> parent<Aspects...>;

        template <class ...Aspects> struct child: public element<"child", ::ht::interface::Child, Aspects...> {
            using element<"child", ::ht::interface::Child , Aspects...>::element;
        };
        template <class ...Aspects> child(Aspects&&...) -> child<Aspects...>;

        template <class ...Aspects> struct child2: public element<"child", ::ht::interface::Child2, Aspects...> {
            using element<"child", ::ht::interface::Child2 , Aspects...>::element;
        };
        template <class ...Aspects> child2(Aspects&&...) -> child2<Aspects...>;

    }
}

#endif //HTELEM_SIMPLE_ELEMENTS_H
