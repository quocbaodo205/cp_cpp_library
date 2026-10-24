#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <functional>
#include <utility>
#include <vector>

namespace cp {

constexpr std::uint64_t binomial_u64(unsigned n, unsigned k) {
    if (k > n) {
        return 0;
    }
    k = std::min(k, n - k);
    std::uint64_t result = 1;
    for (unsigned i = 1; i <= k; ++i) {
        result = result * (n - k + i) / i;
    }
    return result;
}

constexpr std::uint64_t weak_compositions(unsigned parts, unsigned total) {
    if (parts == 0) {
        return total == 0 ? 1 : 0;
    }
    return binomial_u64(total + parts - 1, parts - 1);
}

constexpr std::uint64_t compositions(unsigned parts, unsigned total) {
    if (parts == 0) {
        return total == 0 ? 1 : 0;
    }
    return total < parts ? 0 : binomial_u64(total - 1, parts - 1);
}

constexpr std::uint64_t non_consecutive_selections(unsigned n, unsigned k, unsigned minimum_gap = 1) {
    if (k == 0) {
        return 1;
    }
    const std::uint64_t removed = static_cast<std::uint64_t>(minimum_gap) * (k - 1);
    return removed > n ? 0 : binomial_u64(static_cast<unsigned>(n - removed), k);
}

inline std::uint64_t binomial_mod_prime_small(std::uint64_t n, std::uint64_t k, std::uint64_t prime) {
    if (k > n) {
        return 0;
    }
    k = std::min(k, n - k);
    std::uint64_t result = 1;
    for (std::uint64_t value = 1; value <= k; ++value) {
        result = static_cast<std::uint64_t>(static_cast<__uint128_t>(result) * (n - k + value) % prime);
        std::uint64_t inverse = 1;
        std::uint64_t base = value;
        std::uint64_t exponent = prime - 2;
        while (exponent > 0) {
            if ((exponent & 1U) != 0) {
                inverse = static_cast<std::uint64_t>(static_cast<__uint128_t>(inverse) * base % prime);
            }
            base = static_cast<std::uint64_t>(static_cast<__uint128_t>(base) * base % prime);
            exponent >>= 1U;
        }
        result = static_cast<std::uint64_t>(static_cast<__uint128_t>(result) * inverse % prime);
    }
    return result;
}

inline std::uint64_t binomial_mod_lucas(std::uint64_t n, std::uint64_t k, std::uint64_t prime) {
    std::uint64_t result = 1;
    while (n > 0 || k > 0) {
        result = static_cast<std::uint64_t>(static_cast<__uint128_t>(result) *
                 binomial_mod_prime_small(n % prime, k % prime, prime) % prime);
        n /= prime;
        k /= prime;
    }
    return result;
}

template <class Mint>
class Combinatorics {
public:
    explicit Combinatorics(std::size_t maximum = 0) : factorial_(1, Mint{1}), inverse_factorial_(1, Mint{1}) {
        reserve(maximum);
    }

    void reserve(std::size_t maximum) {
        if (maximum + 1 <= factorial_.size()) {
            return;
        }
        const std::size_t old_size = factorial_.size();
        factorial_.resize(maximum + 1);
        for (std::size_t value = old_size; value <= maximum; ++value) {
            factorial_[value] = factorial_[value - 1] * Mint(value);
        }
        inverse_factorial_.resize(maximum + 1);
        inverse_factorial_[maximum] = factorial_[maximum].inv();
        for (std::size_t value = maximum; value >= old_size; --value) {
            inverse_factorial_[value - 1] = inverse_factorial_[value] * Mint(value);
        }
    }

    Mint factorial(std::size_t n) {
        reserve(n);
        return factorial_[n];
    }

    Mint inverse_factorial(std::size_t n) {
        reserve(n);
        return inverse_factorial_[n];
    }

    Mint binomial(std::size_t n, std::size_t k) {
        if (k > n) {
            return Mint{};
        }
        reserve(n);
        return factorial_[n] * inverse_factorial_[k] * inverse_factorial_[n - k];
    }

    Mint permutation(std::size_t n, std::size_t k) {
        if (k > n) {
            return Mint{};
        }
        reserve(n);
        return factorial_[n] * inverse_factorial_[n - k];
    }

    Mint multiset(std::size_t types, std::size_t count) {
        if (types == 0) {
            return count == 0 ? Mint{1} : Mint{};
        }
        return binomial(types + count - 1, count);
    }

private:
    std::vector<Mint> factorial_;
    std::vector<Mint> inverse_factorial_;
};

struct PermutationCycles {
    std::vector<std::vector<int>> cycles;
    std::vector<int> cycle_of;
    std::vector<int> position;
};

inline PermutationCycles split_cycles(const std::vector<int>& permutation) {
    const int n = static_cast<int>(permutation.size());
    PermutationCycles result{{}, std::vector<int>(permutation.size(), -1), std::vector<int>(permutation.size(), -1)};
    std::vector<bool> seen(permutation.size());
    for (int start = 0; start < n; ++start) {
        if (seen[start]) {
            continue;
        }
        const int cycle_id = static_cast<int>(result.cycles.size());
        result.cycles.emplace_back();
        for (int current = start; !seen[current]; current = permutation[current]) {
            assert(0 <= current && current < n);
            seen[current] = true;
            result.cycle_of[current] = cycle_id;
            result.position[current] = static_cast<int>(result.cycles.back().size());
            result.cycles.back().push_back(current);
        }
    }
    return result;
}

inline std::vector<int> permutation_power(const std::vector<int>& permutation, std::uint64_t exponent) {
    const PermutationCycles split = split_cycles(permutation);
    std::vector<int> result(permutation.size());
    for (std::size_t value = 0; value < permutation.size(); ++value) {
        const auto& cycle = split.cycles[split.cycle_of[value]];
        result[value] = cycle[(split.position[value] + exponent % cycle.size()) % cycle.size()];
    }
    return result;
}

template <class Mint, class Visitor>
void enumerate_cycle_types(std::size_t n, Combinatorics<Mint>& combinatorics, Visitor&& visitor) {
    const Mint n_factorial = combinatorics.factorial(n);
    const auto visit = [&](auto&& self, std::size_t maximum_length, std::size_t remaining,
                           __uint128_t current_lcm, Mint denominator) -> void {
        if (remaining == 0) {
            std::invoke(visitor, current_lcm, denominator, n_factorial);
            return;
        }
        if (maximum_length == 0) {
            return;
        }
        const std::size_t length = std::min(maximum_length, remaining);
        for (std::size_t count = remaining / length + 1; count-- > 0;) {
            const __uint128_t next_lcm = count == 0
                ? current_lcm
                : std::lcm(current_lcm, static_cast<__uint128_t>(length));
            const Mint next_denominator = denominator * Mint(length).pow(count) * combinatorics.factorial(count);
            self(self, length - 1, remaining - length * count, next_lcm, next_denominator);
        }
    };
    visit(visit, n, n, __uint128_t{1}, Mint{1});
}

}  // namespace cp

