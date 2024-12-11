#ifndef HTELEM_BRAID_H
#define HTELEM_BRAID_H

#include <algorithm>
#include <functional>
#include <ranges>
#include <type_traits>

namespace ht {
struct empty_strand {};

template <class L, class R> struct braid;

namespace detail {
    template <class T> struct is_braid: std::false_type {};
    template <class L, class R> struct is_braid<braid<L, R>>: std::true_type {};
    template <class T> constexpr bool is_braid_v = is_braid<std::decay_t<T>>::value;

    template <class T> struct count_braid {
        static constexpr std::size_t value = 1;
    };
    template <> struct count_braid<empty_strand> {
        static constexpr std::size_t value = 0;
    };
    template <class L, class R> struct count_braid<braid<L, R>> {
        static constexpr auto value = count_braid<std::decay_t<L>>::value + count_braid<std::decay_t<R>>::value;
    };
} // namespace detail

template <class T> struct braid_common_type
    : std::conditional_t<!detail::is_braid_v<T>, std::type_identity<T>, std::type_identity_t<empty_strand>> {};
template <class L> struct braid_common_type<braid<L, empty_strand>>: std::type_identity<L> {};
template <class L, class R> struct braid_common_type<braid<L, R>>
    : std::common_type<std::conditional_t<detail::is_braid_v<L>, typename braid_common_type<std::decay_t<L>>::type,
                                          std::decay_t<L>>,
                       std::conditional_t<detail::is_braid_v<R>, typename braid_common_type<std::decay_t<R>>::type,
                                          std::decay_t<R>>> {};

template <class T, class = void> constexpr bool is_uniform_braid_v = false;
template <class L, class R>
constexpr bool is_uniform_braid_v<braid<L, R>, std::void_t<typename braid_common_type<braid<L, R>>::type>> = true;

template <class L, class R = empty_strand> struct braid {
    static constexpr bool is_uniform = is_uniform_braid_v<braid<L, R>>;

    const L left;
    const R right;

    constexpr explicit braid(L&& _left): left{ std::move(_left) } {}
    constexpr explicit braid(L&& _left, R&& _right): left{ std::move(_left) }, right{ std::move(_right) } {}

    constexpr auto& first() const {
        if constexpr (detail::is_braid_v<L>) {
            return left.first();
        } else {
            return left;
        }
    }

    template <class Func> constexpr void for_each(Func&& func) const {
        if constexpr (detail::is_braid_v<L>) {
            left.for_each(std::forward<Func>(func));
        } else {
            func(left);
        }
        if constexpr (detail::is_braid_v<R>) {
            right.for_each(std::forward<Func>(func));
        } else if constexpr (!std::is_same_v<R, empty_strand>) {
            func(right);
        }
    }

    template <class T, class Func> constexpr auto reduce(Func&& func, T initial) const {
        for_each([&initial, &func](const auto& value) { initial = std::invoke(func, std::move(initial), value); });
        return initial;
    }

    constexpr auto as_tuple() {
        return reduce(
                []<class... T>(std::tuple<T...>&& t, const auto& value) {
                    return std::tuple_cat(std::move(t), std::make_tuple(value));
                },
                std::make_tuple());
    }

    static constexpr auto count() { return detail::count_braid<braid<L, R>>::value; }
};

template <class L, class R1, class R2>
    requires(!detail::is_braid_v<R2>)
constexpr auto operator+(braid<L, R1>&& b, R2&& rhs) {
    return braid{ std::move(b.left), braid{ std::move(b.right), std::move(rhs) } };
}

template <class L, class R>
    requires(!detail::is_braid_v<R>)
constexpr auto operator+(braid<L, empty_strand>&& b, R&& rhs) {
    return braid{ std::move(b.left), std::move(rhs) };
}

template <class L1, class L2, class R>
    requires(!detail::is_braid_v<L1>)
constexpr auto operator+(L1&& lhs, braid<L2, R>&& b) {
    return braid{ std::move(lhs), std::move(b) };
}

template <class L1, class L2>
    requires(!detail::is_braid_v<L1>)
constexpr auto operator+(L1&& lhs, braid<L2, empty_strand>&& b) {
    return braid{ std::move(lhs), std::move(b.left) };
}

template <class L1, class L2, class R1, class R2> constexpr auto operator+(braid<L1, R1>&& lhs, braid<L2, R2>&& rhs) {
    return braid{ std::move(lhs), std::move(rhs) };
}

template <class L, class R>
    requires(is_uniform_braid_v<braid<L, R>>)
constexpr auto braid_to_array(const braid<L, R>& br) {
    using common_t = braid_common_type<braid<L, R>>::type;
    std::array<common_t, detail::count_braid<braid<L, R>>::value> ar{};
    br.for_each([it = ar.begin()](const common_t& value) mutable { *(it++) = value; });
    return ar;
}

template <class C>
concept sv_braid = requires { std::is_same_v<typename braid_common_type<C>::type, std::string_view>; };

template <class L, class R>
    requires(sv_braid<braid<L, R>>)
constexpr std::size_t sv_braid_size(const braid<L, R>& br) {
    return br.reduce([](std::size_t n, const std::string_view& sv) { return n + sv.size(); }, 0);
}

template <std::size_t Len, class L, class R>
    requires(sv_braid<braid<L, R>>)
constexpr auto sv_braid_to_str(const braid<L, R>& br) {
    std::array<char, Len> ar{};
    auto sv_array = braid_to_array(br);
    auto joined = sv_array | std::ranges::views::join;
    std::copy_n(joined.begin(), Len, ar.begin());
    return ar;
}

template <const auto& Br> constexpr auto braid_literal = sv_braid_to_str<sv_braid_size(Br)>(Br);
template <const auto& Br> constexpr auto braid_literal_view = std::string_view{ braid_literal<Br> };
} // namespace ht

#endif
