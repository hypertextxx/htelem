#ifndef HTELEM__UTILITIES_TUPLE_RANGE_H
#define HTELEM__UTILITIES_TUPLE_RANGE_H

#include "type_traits.h"
#include <iterator>

namespace ht {
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

#endif

