#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "cp/math/ntt.hpp"

namespace cp {

template <class Mint, std::uint32_t PrimitiveRoot>
class FormalPowerSeries {
public:
    FormalPowerSeries() = default;
    explicit FormalPowerSeries(std::vector<Mint> coefficients) : coefficients_(std::move(coefficients)) {}
    FormalPowerSeries(std::initializer_list<Mint> coefficients) : coefficients_(coefficients) {}

    std::size_t size() const { return coefficients_.size(); }
    bool empty() const { return coefficients_.empty(); }
    void resize(std::size_t size) { coefficients_.resize(size); }

    Mint& operator[](std::size_t index) { return coefficients_[index]; }
    const Mint& operator[](std::size_t index) const { return coefficients_[index]; }
    const std::vector<Mint>& coefficients() const { return coefficients_; }

    FormalPowerSeries prefix(std::size_t size) const {
        size = std::min(size, coefficients_.size());
        return FormalPowerSeries(std::vector<Mint>(coefficients_.begin(), coefficients_.begin() +
                                                                  static_cast<std::ptrdiff_t>(size)));
    }

    FormalPowerSeries& operator+=(const FormalPowerSeries& other) {
        coefficients_.resize(std::max(size(), other.size()));
        for (std::size_t index = 0; index < other.size(); ++index) {
            coefficients_[index] += other[index];
        }
        return *this;
    }

    FormalPowerSeries& operator-=(const FormalPowerSeries& other) {
        coefficients_.resize(std::max(size(), other.size()));
        for (std::size_t index = 0; index < other.size(); ++index) {
            coefficients_[index] -= other[index];
        }
        return *this;
    }

    FormalPowerSeries& operator*=(const FormalPowerSeries& other) {
        coefficients_ = convolution_ntt<Mint, PrimitiveRoot>(coefficients_, other.coefficients_);
        return *this;
    }

    FormalPowerSeries& operator*=(Mint scalar) {
        for (Mint& coefficient : coefficients_) {
            coefficient *= scalar;
        }
        return *this;
    }

    friend FormalPowerSeries operator+(FormalPowerSeries left, const FormalPowerSeries& right) { return left += right; }
    friend FormalPowerSeries operator-(FormalPowerSeries left, const FormalPowerSeries& right) { return left -= right; }
    friend FormalPowerSeries operator*(FormalPowerSeries left, const FormalPowerSeries& right) { return left *= right; }
    friend FormalPowerSeries operator*(FormalPowerSeries series, Mint scalar) { return series *= scalar; }

    FormalPowerSeries derivative() const {
        if (size() <= 1) {
            return FormalPowerSeries{};
        }
        std::vector<Mint> result(size() - 1);
        for (std::size_t index = 1; index < size(); ++index) {
            result[index - 1] = coefficients_[index] * Mint(index);
        }
        return FormalPowerSeries(std::move(result));
    }

    FormalPowerSeries integral() const {
        std::vector<Mint> result(size() + 1);
        for (std::size_t index = 0; index < size(); ++index) {
            result[index + 1] = coefficients_[index] / Mint(index + 1);
        }
        return FormalPowerSeries(std::move(result));
    }

    FormalPowerSeries inverse(std::size_t degree) const {
        assert(!empty() && coefficients_[0] != Mint{});
        FormalPowerSeries result({coefficients_[0].inv()});
        for (std::size_t length = 1; length < degree; length <<= 1U) {
            const std::size_t next = std::min(degree, length * 2);
            FormalPowerSeries current = prefix(next);
            FormalPowerSeries product = current * result;
            product.resize(next);
            for (Mint& coefficient : product.coefficients_) {
                coefficient = -coefficient;
            }
            product[0] += Mint{2};
            result *= product;
            result.resize(next);
        }
        result.resize(degree);
        return result;
    }

    FormalPowerSeries logarithm(std::size_t degree) const {
        assert(!empty() && coefficients_[0] == Mint{1});
        FormalPowerSeries result = derivative() * inverse(degree);
        if (degree > 0) {
            result.resize(degree - 1);
        } else {
            result.resize(0);
        }
        result = result.integral();
        result.resize(degree);
        return result;
    }

    FormalPowerSeries exponential(std::size_t degree) const {
        assert(empty() || coefficients_[0] == Mint{});
        FormalPowerSeries result({Mint{1}});
        for (std::size_t length = 1; length < degree; length <<= 1U) {
            const std::size_t next = std::min(degree, length * 2);
            FormalPowerSeries correction = prefix(next) - result.logarithm(next);
            correction.resize(next);
            correction[0] += Mint{1};
            result *= correction;
            result.resize(next);
        }
        result.resize(degree);
        return result;
    }

    FormalPowerSeries pow(std::uint64_t exponent, std::size_t degree) const {
        if (degree == 0) {
            return FormalPowerSeries{};
        }
        if (exponent == 0) {
            std::vector<Mint> result(degree);
            result[0] = Mint{1};
            return FormalPowerSeries(std::move(result));
        }
        std::size_t leading = 0;
        while (leading < size() && coefficients_[leading] == Mint{}) {
            ++leading;
        }
        if (leading == size() || leading > (degree - 1) / exponent) {
            return FormalPowerSeries(std::vector<Mint>(degree));
        }
        const Mint scale = coefficients_[leading];
        std::vector<Mint> shifted(coefficients_.begin() + static_cast<std::ptrdiff_t>(leading), coefficients_.end());
        for (Mint& coefficient : shifted) {
            coefficient /= scale;
        }
        FormalPowerSeries normalized(std::move(shifted));
        FormalPowerSeries powered = (normalized.logarithm(degree - leading * exponent) * Mint(exponent))
                                        .exponential(degree - leading * exponent);
        powered *= scale.pow(exponent);
        std::vector<Mint> result(degree);
        const std::size_t offset = leading * exponent;
        for (std::size_t index = 0; index < powered.size() && index + offset < degree; ++index) {
            result[index + offset] = powered[index];
        }
        return FormalPowerSeries(std::move(result));
    }

    std::optional<FormalPowerSeries> square_root(std::size_t degree) const {
        if (degree == 0) {
            return FormalPowerSeries{};
        }
        std::size_t leading = 0;
        while (leading < size() && coefficients_[leading] == Mint{}) {
            ++leading;
        }
        if (leading == size()) {
            return FormalPowerSeries(std::vector<Mint>(degree));
        }
        if ((leading & 1U) != 0) {
            return std::nullopt;
        }
        const auto scalar_root = modular_square_root(coefficients_[leading]);
        if (!scalar_root.has_value()) {
            return std::nullopt;
        }
        const std::size_t offset = leading / 2;
        if (offset >= degree) {
            return FormalPowerSeries(std::vector<Mint>(degree));
        }
        const std::size_t needed = degree - offset;
        const Mint inverse_leading = coefficients_[leading].inv();
        std::vector<Mint> normalized_coefficients;
        normalized_coefficients.reserve(std::min(needed, size() - leading));
        for (std::size_t index = leading; index < size() && normalized_coefficients.size() < needed; ++index) {
            normalized_coefficients.push_back(coefficients_[index] * inverse_leading);
        }
        FormalPowerSeries normalized(std::move(normalized_coefficients));
        FormalPowerSeries result({Mint{1}});
        const Mint inverse_two = Mint{2}.inv();
        for (std::size_t length = 1; length < needed; length <<= 1U) {
            const std::size_t next = std::min(needed, length * 2);
            FormalPowerSeries quotient = normalized.prefix(next) * result.inverse(next);
            quotient.resize(next);
            result.resize(next);
            result += quotient;
            result *= inverse_two;
            result.resize(next);
        }
        std::vector<Mint> shifted(degree);
        for (std::size_t index = 0; index < result.size() && index + offset < degree; ++index) {
            shifted[index + offset] = result[index] * *scalar_root;
        }
        return FormalPowerSeries(std::move(shifted));
    }

    static FormalPowerSeries sum_pairwise_convolution(
        const std::vector<FormalPowerSeries>& series, std::size_t degree) {
        FormalPowerSeries sum;
        FormalPowerSeries sum_of_squares;
        for (const auto& current : series) {
            sum += current;
            sum_of_squares += current * current;
        }
        FormalPowerSeries result = sum * sum - sum_of_squares;
        result *= Mint{2}.inv();
        result.resize(degree);
        return result;
    }

private:
    std::vector<Mint> coefficients_;

    static std::optional<Mint> modular_square_root(Mint value) {
        const std::uint64_t modulus = Mint::mod();
        if (value == Mint{}) {
            return Mint{};
        }
        if (modulus == 2) {
            return value;
        }
        if (value.pow((modulus - 1) / 2) != Mint{1}) {
            return std::nullopt;
        }
        Mint root;
        if (modulus % 4 == 3) {
            root = value.pow((modulus + 1) / 4);
        } else {
            std::uint64_t odd = modulus - 1;
            unsigned shifts = 0;
            while ((odd & 1U) == 0) {
                odd >>= 1U;
                ++shifts;
            }
            Mint non_residue = 2;
            while (non_residue.pow((modulus - 1) / 2) == Mint{1}) {
                non_residue += Mint{1};
            }
            Mint c = non_residue.pow(odd);
            Mint x = value.pow((odd + 1) / 2);
            Mint t = value.pow(odd);
            unsigned m = shifts;
            while (t != Mint{1}) {
                unsigned index = 1;
                Mint square = t * t;
                while (index < m && square != Mint{1}) {
                    square *= square;
                    ++index;
                }
                if (index == m) {
                    return std::nullopt;
                }
                const Mint factor = c.pow(std::uint64_t{1} << (m - index - 1));
                x *= factor;
                c = factor * factor;
                t *= c;
                m = index;
            }
            root = x;
        }
        const Mint other = -root;
        return other.value() < root.value() ? other : root;
    }
};

using Fps998244353 = FormalPowerSeries<ModInt998244353, 3>;

}  // namespace cp

