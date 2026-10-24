#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cp {

struct ExtendedGcdResult {
    std::int64_t gcd;
    std::int64_t x;
    std::int64_t y;
};

inline ExtendedGcdResult extended_gcd(std::int64_t a, std::int64_t b) {
    std::int64_t old_x = 1;
    std::int64_t x = 0;
    std::int64_t old_y = 0;
    std::int64_t y = 1;
    while (b != 0) {
        const std::int64_t quotient = a / b;
        a -= quotient * b;
        std::swap(a, b);
        old_x -= quotient * x;
        std::swap(old_x, x);
        old_y -= quotient * y;
        std::swap(old_y, y);
    }
    if (a < 0) {
        a = -a;
        old_x = -old_x;
        old_y = -old_y;
    }
    return {a, old_x, old_y};
}

inline std::optional<std::uint64_t> mod_inverse(std::uint64_t value, std::uint64_t modulus) {
    if (modulus == 0 || value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
        modulus > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return std::nullopt;
    }
    const auto result = extended_gcd(static_cast<std::int64_t>(value), static_cast<std::int64_t>(modulus));
    if (result.gcd != 1) {
        return std::nullopt;
    }
    const std::int64_t reduced = result.x % static_cast<std::int64_t>(modulus);
    return static_cast<std::uint64_t>(reduced < 0 ? reduced + static_cast<std::int64_t>(modulus) : reduced);
}

inline std::uint64_t add_mod(std::uint64_t lhs, std::uint64_t rhs, std::uint64_t modulus) {
    assert(modulus > 0);
    return static_cast<std::uint64_t>((static_cast<__uint128_t>(lhs) + rhs) % modulus);
}

inline std::uint64_t multiply_mod(std::uint64_t lhs, std::uint64_t rhs, std::uint64_t modulus) {
    assert(modulus > 0);
    return static_cast<std::uint64_t>(static_cast<__uint128_t>(lhs) * rhs % modulus);
}

inline std::uint64_t power_mod(std::uint64_t base, std::uint64_t exponent, std::uint64_t modulus) {
    assert(modulus > 0);
    std::uint64_t result = 1 % modulus;
    base %= modulus;
    while (exponent > 0) {
        if ((exponent & 1U) != 0) {
            result = multiply_mod(result, base, modulus);
        }
        base = multiply_mod(base, base, modulus);
        exponent >>= 1U;
    }
    return result;
}

inline std::uint64_t factorial_prime_valuation(std::uint64_t n, std::uint64_t prime) {
    assert(prime >= 2);
    std::uint64_t result = 0;
    while (n > 0) {
        n /= prime;
        result += n;
    }
    return result;
}

inline __int128 floor_sum(std::uint64_t n, std::uint64_t modulus, std::int64_t a, std::int64_t b) {
    assert(modulus > 0);
    const auto floor_div128 = [](__int128 numerator, __int128 denominator) {
        __int128 quotient = numerator / denominator;
        const __int128 remainder = numerator % denominator;
        if (remainder < 0) {
            --quotient;
        }
        return quotient;
    };
    __int128 answer = 0;
    const __int128 m = modulus;
    __int128 aa = a;
    __int128 bb = b;
    const __int128 a_quotient = floor_div128(aa, m);
    const __int128 b_quotient = floor_div128(bb, m);
    answer += static_cast<__int128>(n) * (n - 1) / 2 * a_quotient;
    answer += static_cast<__int128>(n) * b_quotient;
    aa -= a_quotient * m;
    bb -= b_quotient * m;

    std::uint64_t unsigned_a = static_cast<std::uint64_t>(aa);
    std::uint64_t unsigned_b = static_cast<std::uint64_t>(bb);
    std::uint64_t unsigned_m = modulus;
    while (true) {
        if (unsigned_a >= unsigned_m) {
            answer += static_cast<__int128>(n - 1) * n * (unsigned_a / unsigned_m) / 2;
            unsigned_a %= unsigned_m;
        }
        if (unsigned_b >= unsigned_m) {
            answer += static_cast<__int128>(n) * (unsigned_b / unsigned_m);
            unsigned_b %= unsigned_m;
        }
        const __uint128_t maximum = static_cast<__uint128_t>(unsigned_a) * n + unsigned_b;
        if (maximum < unsigned_m) {
            break;
        }
        n = static_cast<std::uint64_t>(maximum / unsigned_m);
        unsigned_b = static_cast<std::uint64_t>(maximum % unsigned_m);
        std::swap(unsigned_a, unsigned_m);
    }
    return answer;
}

struct FloorQuotientRange {
    std::uint64_t left;
    std::uint64_t right;
    std::uint64_t quotient;
};

struct WeightedPrefixInterval {
    std::uint64_t end = 0;
    std::uint64_t copies = 0;

    __uint128_t value_count() const {
        return static_cast<__uint128_t>(end) * copies;
    }

    std::vector<WeightedPrefixInterval> apply_mod(std::uint64_t modulus) const {
        assert(modulus > 0);
        if (end == 0 || copies == 0) {
            return {};
        }
        if (end <= modulus) {
            return {*this};
        }
        std::vector<WeightedPrefixInterval> result{{modulus, copies * (end / modulus)}};
        if (end % modulus != 0) {
            result.push_back({end % modulus, copies});
        }
        return result;
    }
};

inline std::vector<FloorQuotientRange> floor_quotient_ranges(std::uint64_t n) {
    std::vector<FloorQuotientRange> ranges;
    for (std::uint64_t left = 1; left <= n;) {
        const std::uint64_t quotient = n / left;
        const std::uint64_t right = n / quotient;
        ranges.push_back({left, right, quotient});
        left = right + 1;
    }
    return ranges;
}

inline std::vector<int> mobius_sieve(std::size_t limit) {
    std::vector<int> mobius(limit + 1);
    std::vector<int> primes;
    std::vector<bool> composite(limit + 1);
    if (limit >= 1) {
        mobius[1] = 1;
    }
    for (std::size_t value = 2; value <= limit; ++value) {
        if (!composite[value]) {
            primes.push_back(static_cast<int>(value));
            mobius[value] = -1;
        }
        for (int prime : primes) {
            const std::size_t product = value * static_cast<std::size_t>(prime);
            if (product > limit) {
                break;
            }
            composite[product] = true;
            if (value % static_cast<std::size_t>(prime) == 0) {
                mobius[product] = 0;
                break;
            }
            mobius[product] = -mobius[value];
        }
    }
    return mobius;
}

inline std::vector<std::uint64_t> totient_sieve(std::size_t limit) {
    std::vector<std::uint64_t> result(limit + 1);
    std::iota(result.begin(), result.end(), std::uint64_t{0});
    for (std::size_t prime = 2; prime <= limit; ++prime) {
        if (result[prime] == prime) {
            for (std::size_t multiple = prime; multiple <= limit; multiple += prime) {
                result[multiple] = result[multiple] / prime * (prime - 1);
            }
        }
    }
    if (limit >= 1) {
        result[1] = 1;
    }
    return result;
}

struct PrimePowerFactor {
    std::uint64_t prime;
    unsigned exponent;

    auto operator<=>(const PrimePowerFactor&) const = default;
};

inline std::vector<std::uint64_t> divisors_from_factors(const std::vector<PrimePowerFactor>& factors) {
    std::vector<std::uint64_t> result{1};
    for (const PrimePowerFactor& factor : factors) {
        const std::size_t old_size = result.size();
        std::uint64_t power = 1;
        for (unsigned exponent = 1; exponent <= factor.exponent; ++exponent) {
            power *= factor.prime;
            for (std::size_t index = 0; index < old_size; ++index) {
                result.push_back(result[index] * power);
            }
        }
    }
    return result;
}

inline std::vector<std::uint64_t> square_free_divisors_from_factors(
    const std::vector<PrimePowerFactor>& factors) {
    std::vector<std::uint64_t> result{1};
    for (const PrimePowerFactor& factor : factors) {
        const std::size_t old_size = result.size();
        for (std::size_t index = 0; index < old_size; ++index) {
            result.push_back(result[index] * factor.prime);
        }
    }
    return result;
}

inline std::uint64_t multiplicative_order_prime(std::uint64_t value, std::uint64_t prime,
                                                const std::vector<PrimePowerFactor>& factors_of_prime_minus_one) {
    assert(std::gcd(value, prime) == 1);
    std::uint64_t order = prime - 1;
    for (const PrimePowerFactor& factor : factors_of_prime_minus_one) {
        for (unsigned exponent = 0; exponent < factor.exponent && order % factor.prime == 0; ++exponent) {
            if (power_mod(value, order / factor.prime, prime) != 1) {
                break;
            }
            order /= factor.prime;
        }
    }
    return order;
}

inline long long choose2(std::size_t count) {
    return static_cast<long long>(count * (count - (count > 0 ? 1U : 0U)) / 2);
}

inline long long count_coprime_pairs_up_to(std::size_t limit, const std::vector<int>& mobius) {
    assert(mobius.size() > limit);
    long long result = 0;
    for (std::size_t divisor = 1; divisor <= limit; ++divisor) {
        result += static_cast<long long>(mobius[divisor]) * choose2(limit / divisor);
    }
    return result;
}

struct RadixBlock {
    std::uint64_t power = 1;
    std::uint64_t value = 0;

    static RadixBlock digit(std::uint64_t digit_value, std::uint64_t base, std::uint64_t modulus) {
        return {base % modulus, digit_value % modulus};
    }

    static RadixBlock from_integer(std::uint64_t number, std::uint64_t base, std::uint64_t modulus) {
        if (number == 0) {
            return digit(0, base, modulus);
        }
        RadixBlock result;
        std::uint64_t place = 1;
        while (place <= number / base) {
            place *= base;
        }
        while (place > 0) {
            result = concatenate(result, digit(number / place % base, base, modulus), modulus);
            place /= base;
        }
        return result;
    }

    static RadixBlock concatenate(const RadixBlock& left, const RadixBlock& right, std::uint64_t modulus) {
        return {multiply_mod(left.power, right.power, modulus),
                add_mod(multiply_mod(left.value, right.power, modulus), right.value, modulus)};
    }

    RadixBlock repeat(std::uint64_t copies, std::uint64_t modulus) const {
        RadixBlock base = *this;
        RadixBlock result;
        while (copies > 0) {
            if ((copies & 1U) != 0) {
                result = concatenate(result, base, modulus);
            }
            base = concatenate(base, base, modulus);
            copies >>= 1U;
        }
        return result;
    }
};

inline std::optional<std::pair<std::int64_t, std::int64_t>> chinese_remainder(
    std::int64_t remainder1, std::int64_t modulus1, std::int64_t remainder2, std::int64_t modulus2) {
    assert(modulus1 > 0 && modulus2 > 0);
    const auto gcd = extended_gcd(modulus1, modulus2);
    const std::int64_t difference = remainder2 - remainder1;
    if (difference % gcd.gcd != 0) {
        return std::nullopt;
    }
    const std::int64_t second_reduced = modulus2 / gcd.gcd;
    const __int128 multiplier = static_cast<__int128>(difference / gcd.gcd) * gcd.x;
    const std::int64_t normalized = static_cast<std::int64_t>((multiplier % second_reduced + second_reduced) % second_reduced);
    const std::int64_t lcm = modulus1 / gcd.gcd * modulus2;
    const std::int64_t result = static_cast<std::int64_t>((static_cast<__int128>(remainder1) +
        static_cast<__int128>(modulus1) * normalized) % lcm);
    return std::pair{result < 0 ? result + lcm : result, lcm};
}

inline std::optional<std::uint64_t> baby_step_giant_step(std::uint64_t base, std::uint64_t target,
                                                         std::uint64_t modulus) {
    if (modulus == 0) {
        return std::nullopt;
    }
    if (modulus == 1) {
        return 0;
    }
    base %= modulus;
    target %= modulus;
    if (std::gcd(base, modulus) != 1) {
        return std::nullopt;
    }
    const std::uint64_t step = static_cast<std::uint64_t>(std::sqrt(static_cast<long double>(modulus))) + 1;
    std::unordered_map<std::uint64_t, std::uint64_t> baby;
    std::uint64_t current = 1;
    for (std::uint64_t exponent = 0; exponent < step; ++exponent) {
        baby.try_emplace(current, exponent);
        current = multiply_mod(current, base, modulus);
    }
    const auto inverse_stride = mod_inverse(power_mod(base, step, modulus), modulus);
    assert(inverse_stride.has_value());
    std::optional<std::uint64_t> best;
    current = target;
    for (std::uint64_t giant_step = 0; giant_step <= step; ++giant_step) {
        if (const auto iterator = baby.find(current); iterator != baby.end()) {
            const std::uint64_t candidate = giant_step * step + iterator->second;
            if (!best.has_value() || candidate < *best) {
                best = candidate;
            }
        }
        current = multiply_mod(current, *inverse_stride, modulus);
    }
    return best;
}

inline std::optional<std::uint64_t> multiplicative_order(std::uint64_t base, std::uint64_t modulus) {
    if (modulus == 1) {
        return 1;
    }
    const auto inverse = mod_inverse(base % modulus, modulus);
    if (!inverse.has_value()) {
        return std::nullopt;
    }
    const auto exponent = baby_step_giant_step(base, *inverse, modulus);
    return exponent.has_value() ? std::optional<std::uint64_t>(*exponent + 1) : std::nullopt;
}

}  // namespace cp

