#ifndef HTELEM__UTILITIES_TYPE_TRAITS_H
#define HTELEM__UTILITIES_TYPE_TRAITS_H

#include "pack_index.h"

namespace ht::detail {
template <class From, class... To> struct first_convertible_to;
template <class From> struct first_convertible_to<From>: std::false_type { };
template <class From, class To, class... Tail> struct first_convertible_to<From, To, Tail...>
    : std::conditional_t<std::is_convertible_v<From, To>, std::type_identity<To>, first_convertible_to<From, Tail...>> {
};
template <class From, class... To> using first_convertible_to_t = first_convertible_to<From, To...>::type;

template <template <class> class Predicate, class Seq, std::size_t J, class... T> struct filtered_index_sequence_helper;
template <template <class> class Predicate, std::size_t... I, class... T>
struct filtered_index_sequence_helper<Predicate, std::index_sequence<I...>, sizeof...(T), T...> {
    using type = std::index_sequence<I...>;
};
template <template <class> class Predicate, std::size_t... I, std::size_t J, class... T>
struct filtered_index_sequence_helper<Predicate, std::index_sequence<I...>, J, T...>
    : std::conditional_t<Predicate<_ht_pack_index(T, J)>::value,
              filtered_index_sequence_helper<Predicate, std::index_sequence<I..., J>, J + 1, T...>,
              filtered_index_sequence_helper<Predicate, std::index_sequence<I...>, J + 1, T...>> { };

template <template <class> class Predicate, class... T> using filtered_index_sequence =
        filtered_index_sequence_helper<Predicate, std::index_sequence<>, 0, T...>::type;

template <template <class> class Predicate, class... T> struct filter_types;
template <template <class> class Predicate> struct filter_types<Predicate> {
    using type = std::tuple<>;
};
template <template <class> class Predicate, class T, class... Tail> struct filter_types<Predicate, T, Tail...> {
    using type = typename std::conditional<Predicate<T>::value,
            decltype(std::tuple_cat(std::declval<std::tuple<T>>(),
                    std::declval<typename filter_types<Predicate, Tail...>::type>())),
            typename filter_types<Predicate, Tail...>::type>::type;
};

} // namespace ht::detail

#endif

