#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <vector>

namespace cp {

inline void fft(std::vector<std::complex<double>>& values, bool inverse = false) {
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
        const double angle = 2.0 * std::numbers::pi / static_cast<double>(length) * (inverse ? -1.0 : 1.0);
        const std::complex<double> root(std::cos(angle), std::sin(angle));
        for (std::size_t left = 0; left < n; left += length) {
            std::complex<double> power = 1.0;
            for (std::size_t offset = 0; offset < length / 2; ++offset) {
                const auto even = values[left + offset];
                const auto odd = values[left + offset + length / 2] * power;
                values[left + offset] = even + odd;
                values[left + offset + length / 2] = even - odd;
                power *= root;
            }
        }
    }
    if (inverse) {
        for (auto& value : values) {
            value /= static_cast<double>(n);
        }
    }
}

inline std::vector<double> convolution(const std::vector<double>& lhs, const std::vector<double>& rhs) {
    if (lhs.empty() || rhs.empty()) {
        return {};
    }
    const std::size_t result_size = lhs.size() + rhs.size() - 1;
    const std::size_t size = std::bit_ceil(result_size);
    std::vector<std::complex<double>> left(size);
    std::vector<std::complex<double>> right(size);
    std::copy(lhs.begin(), lhs.end(), left.begin());
    std::copy(rhs.begin(), rhs.end(), right.begin());
    fft(left);
    fft(right);
    for (std::size_t index = 0; index < size; ++index) {
        left[index] *= right[index];
    }
    fft(left, true);
    std::vector<double> result(result_size);
    for (std::size_t index = 0; index < result_size; ++index) {
        result[index] = left[index].real();
    }
    return result;
}

inline std::vector<long long> convolution_ll(const std::vector<long long>& lhs, const std::vector<long long>& rhs) {
    std::vector<double> left(lhs.begin(), lhs.end());
    std::vector<double> right(rhs.begin(), rhs.end());
    const auto raw = convolution(left, right);
    std::vector<long long> result(raw.size());
    std::transform(raw.begin(), raw.end(), result.begin(), [](double value) { return std::llround(value); });
    return result;
}

}  // namespace cp

