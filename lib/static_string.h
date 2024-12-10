#ifndef HTELEM_STATIC_STRING_H
#define HTELEM_STATIC_STRING_H

#include "fmt/base.h"
#include "fmt/format.h"
#include <algorithm>
#include <string_view>

namespace ht {

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    template <std::size_t N> struct static_string {
        static constexpr std::size_t _size = N;
        char data[N]{ };

        constexpr explicit(false) static_string(const std::array<char, N+1>& _data) {
            std::copy_n(_data.begin(), N, data);
        }

        constexpr explicit(false) static_string(const char (&_data)[N + 1]) {
            std::copy_n(_data, N, data);
        }

        constexpr explicit(false) static_string(std::string_view sv) {
            std::copy_n(sv.cbegin(), N, data);
        }

        constexpr explicit(false) operator std::string_view() const {
            return std::string_view { data, N };
        }

        constexpr bool operator==(std::string_view sv) const {
            return sv == std::string_view { data, N };
        }

        [[nodiscard]] constexpr std::size_t size() const {
            return N;
        }

        template <std::size_t R> constexpr static_string<N + R> operator+(const static_string<R>& rhs) const {
            return static_string<N + R>{ *this, rhs };
        }

        template <std::size_t L, std::size_t R> constexpr static_string<L + R>(const static_string<L>& lhs, const static_string<R>& rhs) {
            auto it = std::copy_n(lhs.data, L, data);
            std::copy_n(rhs.data, R, it);
        }
    };
#pragma clang diagnostic pop

    template <std::size_t N> static_string(const char (&_data)[N]) -> static_string<N-1>;
    template <std::size_t N> static_string(const std::array<char, N>&) -> static_string<N-1>;
}

template <std::size_t N> struct fmt::formatter<ht::static_string<N>>: fmt::formatter<std::string_view> {
    template <class Ctx> constexpr auto format (const ht::static_string<N>& ss, Ctx& ctx) const {
        return fmt::formatter<std::string_view>::format(static_cast<std::string_view>(ss), ctx);
    }
};

#endif //HTELEM_STATIC_STRING_H
