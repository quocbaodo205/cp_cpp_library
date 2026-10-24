#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "cp/math/number_theory.hpp"

namespace cp {

class PrimeSieve {
  public:
    explicit PrimeSieve(std::size_t limit) : smallest_factor_(limit + 1) {
        for (std::size_t value = 2; value <= limit; ++value) {
            if (smallest_factor_[value] == 0) {
                smallest_factor_[value] = static_cast<int>(value);
                primes_.push_back(static_cast<int>(value));
            }
            for (int prime : primes_) {
                const std::size_t product = value * static_cast<std::size_t>(prime);
                if (product > limit || prime > smallest_factor_[value]) {
                    break;
                }
                smallest_factor_[product] = prime;
            }
        }
    }

    std::size_t limit() const {
        return smallest_factor_.empty() ? 0 : smallest_factor_.size() - 1;
    }
    const std::vector<int> &primes() const {
        return primes_;
    }

    bool is_prime(std::size_t value) const {
        return value >= 2 && value <= limit() && smallest_factor_[value] == static_cast<int>(value);
    }

    int smallest_prime_factor(std::size_t value) const {
        assert(value >= 2 && value <= limit());
        return smallest_factor_[value];
    }

    std::optional<int> prime_at_most(std::size_t value) const {
        const auto iterator = std::upper_bound(primes_.begin(), primes_.end(), static_cast<int>(value));
        if (iterator == primes_.begin()) {
            return std::nullopt;
        }
        return *std::prev(iterator);
    }

    std::optional<int> prime_at_least(std::size_t value) const {
        const auto iterator = std::lower_bound(primes_.begin(), primes_.end(), static_cast<int>(value));
        return iterator == primes_.end() ? std::nullopt : std::optional<int>(*iterator);
    }

    std::vector<PrimePowerFactor> factorize(std::size_t value) const {
        assert(value >= 1 && value <= limit());
        std::vector<PrimePowerFactor> result;
        while (value > 1) {
            const std::size_t prime = static_cast<std::size_t>(smallest_factor_[value]);
            unsigned exponent = 0;
            do {
                value /= prime;
                ++exponent;
            } while (value > 1 && smallest_factor_[value] == static_cast<int>(prime));
            result.push_back({prime, exponent});
        }
        return result;
    }

    std::vector<int> smallest_factor_;
    std::vector<int> primes_;
};

inline bool is_prime(std::uint64_t value) {
    if (value < 2) {
        return false;
    }
    for (std::uint64_t prime : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL, 19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (value % prime == 0) {
            return value == prime;
        }
    }
    std::uint64_t odd = value - 1;
    unsigned shifts = 0;
    while ((odd & 1U) == 0) {
        odd >>= 1U;
        ++shifts;
    }
    for (std::uint64_t base : {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL}) {
        if (base % value == 0) {
            continue;
        }
        std::uint64_t power = power_mod(base, odd, value);
        if (power == 1 || power == value - 1) {
            continue;
        }
        bool composite = true;
        for (unsigned round = 1; round < shifts; ++round) {
            power = multiply_mod(power, power, value);
            if (power == value - 1) {
                composite = false;
                break;
            }
        }
        if (composite) {
            return false;
        }
    }
    return true;
}

inline std::uint64_t primitive_root_prime(std::uint64_t prime) {
    assert(is_prime(prime));
    if (prime == 2) {
        return 1;
    }
    std::uint64_t remaining = prime - 1;
    std::vector<std::uint64_t> factors;
    for (std::uint64_t factor = 2; factor <= remaining / factor; ++factor) {
        if (remaining % factor != 0) {
            continue;
        }
        factors.push_back(factor);
        while (remaining % factor == 0) {
            remaining /= factor;
        }
    }
    if (remaining > 1) {
        factors.push_back(remaining);
    }
    for (std::uint64_t candidate = 2;; ++candidate) {
        bool valid = true;
        for (std::uint64_t factor : factors) {
            if (power_mod(candidate, (prime - 1) / factor, prime) == 1) {
                valid = false;
                break;
            }
        }
        if (valid) {
            return candidate;
        }
    }
}

} // namespace cp
