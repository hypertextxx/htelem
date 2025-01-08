#ifndef HTELEM_UTIL_H
#define HTELEM_UTIL_H

#include "static_string.h"
#include <iterator>
#include <tuple>
#include <type_traits>

#if __cpp_pack_indexing >= 202311L
#define _ht_pack_index(pack, idx) pack...[idx]
#else
#define _ht_pack_index(pack, idx) std::tuple_element_t<idx, std::tuple<pack...>>
#endif

namespace ht {
namespace detail {
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
} // namespace detail

template <class UT, class Seq> class tuple_range;
template <class UT, std::size_t... I> class tuple_range<UT, std::index_sequence<I...>> {
    using uniform_type = UT;
    using storage_type = std::reference_wrapper<uniform_type>;

    static constexpr auto size = sizeof...(I);

    static constexpr storage_type store(auto& v) { return std::ref(v); }

    std::array<storage_type, size> refs;

    class iterator {
        using wrapped_iterator = decltype(refs.begin());
        wrapped_iterator it;

        static constexpr uniform_type& get(const wrapped_iterator& it) { return (*it).get(); }

    public:
        using value_type = UT;
        using reference = value_type&;
        using difference_type = std::iter_difference_t<wrapped_iterator>;

        constexpr iterator() = default;
        constexpr iterator(wrapped_iterator _it): it{_it} { }

        constexpr reference operator*() const { return get(it); }

        constexpr reference operator[](difference_type n) const { return get(it + n); }

        constexpr auto& operator++() {
            ++it;
            return *this;
        }

        constexpr auto operator++(int) {
            auto prev = *this;
            ++it;
            return prev;
        }

        constexpr auto& operator--() {
            --it;
            return *this;
        }

        constexpr auto operator--(int) {
            auto prev = *this;
            --it;
            return prev;
        }

        constexpr auto& operator+=(difference_type n) {
            it += n;
            return *this;
        }

        constexpr auto& operator-=(difference_type n) {
            it -= n;
            return *this;
        }

        constexpr auto operator+(difference_type n) const { return iterator{it + n}; }

        constexpr auto operator-(difference_type n) const { return iterator{it - n}; }

        friend constexpr auto operator+(difference_type n, const iterator& rhs) { return iterator{n + rhs.it}; }

        friend constexpr auto operator-(difference_type n, const iterator& rhs) { return iterator{n - rhs.it}; }

        constexpr difference_type operator-(const iterator& rhs) const { return it - rhs.it; }

        constexpr bool operator==(const iterator& other) const { return it == other.it; }
        constexpr auto operator<=>(const iterator& other) const { return it <=> other.it; }
    };

public:
    template <class Tuple> constexpr explicit tuple_range(Tuple&& tuple): refs{store(std::get<I>(tuple))...} {};

    constexpr auto begin() { return iterator{refs.begin()}; }
    constexpr auto begin() const { return iterator{refs.begin()}; }
    constexpr auto end() { return iterator{refs.end()}; }
    constexpr auto end() const { return iterator{refs.end()}; }
};

namespace detail {
template <class UT> class tuple_view_helper {
    template <class T> struct is_ut_functor: std::is_same<T, UT> { };

public:
    template <template <class...> class Tuple, class... T> constexpr auto operator()(Tuple<T...>& tuple) {
        return tuple_range<UT, detail::filtered_index_sequence<is_ut_functor, T...>>{tuple};
    }
};
} // namespace detail

template <class UT, class Tuple> constexpr auto make_tuple_range(Tuple&& tuple) {
    return detail::tuple_view_helper<UT>{}(std::forward<Tuple>(tuple));
}

} // namespace ht
#endif // HTELEM_UTIL_H
