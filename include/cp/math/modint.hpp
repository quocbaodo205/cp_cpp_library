#pragma once

#include <cassert>
#include <compare>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <utility>

namespace cp {

template <std::uint32_t Modulus>
class StaticModInt {
    static_assert(Modulus > 0);

public:
    using value_type = std::uint32_t;
    static constexpr value_type mod() { return Modulus; }

    constexpr StaticModInt() = default;

    template <std::integral Integer>
    constexpr StaticModInt(Integer value) {
        if constexpr (std::is_signed_v<Integer>) {
            const __int128 reduced = static_cast<__int128>(value) % static_cast<__int128>(Modulus);
            value_ = static_cast<value_type>(reduced < 0 ? reduced + Modulus : reduced);
        } else {
            value_ = static_cast<value_type>(static_cast<__uint128_t>(value) % Modulus);
        }
    }

    constexpr value_type value() const { return value_; }
    explicit constexpr operator value_type() const { return value_; }

    constexpr StaticModInt& operator+=(const StaticModInt& other) {
        value_ += other.value_;
        if (value_ >= Modulus) {
            value_ -= Modulus;
        }
        return *this;
    }

    constexpr StaticModInt& operator-=(const StaticModInt& other) {
        value_ = value_ >= other.value_ ? value_ - other.value_ : value_ + Modulus - other.value_;
        return *this;
    }

    constexpr StaticModInt& operator*=(const StaticModInt& other) {
        value_ = static_cast<value_type>(static_cast<std::uint64_t>(value_) * other.value_ % Modulus);
        return *this;
    }

    constexpr StaticModInt& operator/=(const StaticModInt& other) { return *this *= other.inv(); }

    constexpr StaticModInt operator+() const { return *this; }
    constexpr StaticModInt operator-() const { return value_ == 0 ? *this : raw(Modulus - value_); }

    friend constexpr StaticModInt operator+(StaticModInt left, const StaticModInt& right) { return left += right; }
    friend constexpr StaticModInt operator-(StaticModInt left, const StaticModInt& right) { return left -= right; }
    friend constexpr StaticModInt operator*(StaticModInt left, const StaticModInt& right) { return left *= right; }
    friend constexpr StaticModInt operator/(StaticModInt left, const StaticModInt& right) { return left /= right; }
    friend constexpr auto operator<=>(const StaticModInt&, const StaticModInt&) = default;

    constexpr StaticModInt pow(std::uint64_t exponent) const {
        StaticModInt base = *this;
        StaticModInt result = 1;
        while (exponent > 0) {
            if ((exponent & 1U) != 0) {
                result *= base;
            }
            base *= base;
            exponent >>= 1U;
        }
        return result;
    }

    constexpr StaticModInt inv() const {
        std::int64_t a = value_;
        std::int64_t b = Modulus;
        std::int64_t x = 1;
        std::int64_t y = 0;
        while (b != 0) {
            const std::int64_t quotient = a / b;
            a -= quotient * b;
            std::swap(a, b);
            x -= quotient * y;
            std::swap(x, y);
        }
        assert(a == 1);
        return StaticModInt(x);
    }

    static constexpr StaticModInt raw(value_type value) {
        StaticModInt result;
        result.value_ = value;
        return result;
    }

    friend std::istream& operator>>(std::istream& input, StaticModInt& value) {
        long long raw_value = 0;
        input >> raw_value;
        value = StaticModInt(raw_value);
        return input;
    }

    friend std::ostream& operator<<(std::ostream& output, const StaticModInt& value) {
        return output << value.value_;
    }

private:
    value_type value_ = 0;
};

using ModInt998244353 = StaticModInt<998244353>;
using ModInt1000000007 = StaticModInt<1000000007>;

}  // namespace cp

