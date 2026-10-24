#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

#include "cp/math/modint.hpp"

namespace cp {

template <class Mint, std::uint32_t PrimitiveRoot>
void ntt(std::vector<Mint>& values, bool inverse = false) {
    const std::size_t n = values.size();
    assert(n == 0 || std::has_single_bit(n));
    for (std::size_t index = 1, reverse = 0; index < n; ++index) {
        std::size_t bit = n >> 1U;
        for (; (reverse & bit) != 0; bit >>= 1U) {
            reverse ^= bit;
        }
        reverse ^= bit;
        if (index < reverse) {
            std::swap(values[index], values[reverse]);
        }
    }
    for (std::size_t length = 2; length <= n; length <<= 1U) {
        assert((Mint::mod() - 1U) % length == 0);
        Mint root = Mint(PrimitiveRoot).pow((Mint::mod() - 1U) / length);
        if (inverse) {
            root = root.inv();
        }
        for (std::size_t left = 0; left < n; left += length) {
            Mint power = 1;
            for (std::size_t offset = 0; offset < length / 2; ++offset) {
                const Mint even = values[left + offset];
                const Mint odd = values[left + offset + length / 2] * power;
                values[left + offset] = even + odd;
                values[left + offset + length / 2] = even - odd;
                power *= root;
            }
        }
    }
    if (inverse && n > 0) {
        const Mint inverse_size = Mint(n).inv();
        for (Mint& value : values) {
            value *= inverse_size;
        }
    }
}

template <class Mint, std::uint32_t PrimitiveRoot>
std::vector<Mint> convolution_ntt(const std::vector<Mint>& lhs, const std::vector<Mint>& rhs) {
    if (lhs.empty() || rhs.empty()) {
        return {};
    }
    if (std::min(lhs.size(), rhs.size()) <= 32) {
        std::vector<Mint> result(lhs.size() + rhs.size() - 1);
        for (std::size_t i = 0; i < lhs.size(); ++i) {
            for (std::size_t j = 0; j < rhs.size(); ++j) {
                result[i + j] += lhs[i] * rhs[j];
            }
        }
        return result;
    }
    const std::size_t result_size = lhs.size() + rhs.size() - 1;
    const std::size_t size = std::bit_ceil(result_size);
    assert((Mint::mod() - 1U) % size == 0);
    std::vector<Mint> left(size);
    std::vector<Mint> right(size);
    std::copy(lhs.begin(), lhs.end(), left.begin());
    std::copy(rhs.begin(), rhs.end(), right.begin());
    ntt<Mint, PrimitiveRoot>(left);
    ntt<Mint, PrimitiveRoot>(right);
    for (std::size_t index = 0; index < size; ++index) {
        left[index] *= right[index];
    }
    ntt<Mint, PrimitiveRoot>(left, true);
    left.resize(result_size);
    return left;
}

inline std::vector<ModInt998244353> convolution(
    const std::vector<ModInt998244353>& lhs, const std::vector<ModInt998244353>& rhs) {
    return convolution_ntt<ModInt998244353, 3>(lhs, rhs);
}

}  // namespace cp

