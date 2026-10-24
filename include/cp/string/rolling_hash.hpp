#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace cp {

class RollingHash {
public:
    using value_type = std::uint64_t;
    static constexpr value_type modulus = (value_type{1} << 61U) - 1;

    explicit RollingHash(std::string_view text, value_type base = 911382323) : powers_(text.size() + 1, 1),
                                                                              prefix_(text.size() + 1) {
        for (std::size_t index = 0; index < text.size(); ++index) {
            powers_[index + 1] = multiply(powers_[index], base);
            prefix_[index + 1] = add(multiply(prefix_[index], base),
                                     static_cast<unsigned char>(text[index]) + 1U);
        }
    }

    value_type hash(std::size_t left, std::size_t right) const {
        assert(left <= right && right < prefix_.size());
        return subtract(prefix_[right], multiply(prefix_[left], powers_[right - left]));
    }

    static value_type concatenate(value_type left_hash, value_type right_hash,
                                                std::size_t right_length, const RollingHash& source) {
        assert(right_length < source.powers_.size());
        return add(multiply(left_hash, source.powers_[right_length]), right_hash);
    }

private:
    std::vector<value_type> powers_;
    std::vector<value_type> prefix_;

    static value_type add(value_type left, value_type right) {
        const value_type result = left + right;
        return result >= modulus ? result - modulus : result;
    }

    static value_type subtract(value_type left, value_type right) {
        return left >= right ? left - right : left + modulus - right;
    }

    static value_type multiply(value_type left, value_type right) {
        const __uint128_t product = static_cast<__uint128_t>(left) * right;
        value_type result = static_cast<value_type>(product >> 61U) + static_cast<value_type>(product & modulus);
        if (result >= modulus) {
            result -= modulus;
        }
        return result;
    }
};

}  // namespace cp

