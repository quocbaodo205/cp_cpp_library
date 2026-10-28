#pragma once

#include <algorithm>
#include <bit>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace cp {

template <class T, class U>
constexpr bool chmin(T& current, U&& candidate) {
    if (candidate < current) {
        current = std::forward<U>(candidate);
        return true;
    }
    return false;
}

template <class T, class U>
constexpr bool chmax(T& current, U&& candidate) {
    if (current < candidate) {
        current = std::forward<U>(candidate);
        return true;
    }
    return false;
}

template <std::integral T>
constexpr T floor_div(T numerator, T denominator) {
    T quotient = numerator / denominator;
    const T remainder = numerator % denominator;
    if (remainder != 0 && ((remainder < 0) != (denominator < 0))) {
        --quotient;
    }
    return quotient;
}

template <std::integral T>
constexpr T ceil_div(T numerator, T denominator) {
    T quotient = numerator / denominator;
    const T remainder = numerator % denominator;
    if (remainder != 0 && ((remainder < 0) == (denominator < 0))) {
        ++quotient;
    }
    return quotient;
}

template <class T, class Exponent, class Operation = std::multiplies<>>
constexpr T binary_power(T base, Exponent exponent, T identity = T{1}, Operation operation = {}) {
    T result = identity;
    while (exponent > 0) {
        if ((exponent & 1) != 0) {
            result = operation(result, base);
        }
        base = operation(base, base);
        exponent >>= 1;
    }
    return result;
}

template <class Range, class T, class Compare = std::less<>>
std::size_t lower_bound_index(const Range& range, const T& value, Compare compare = {}) {
    return static_cast<std::size_t>(std::lower_bound(range.begin(), range.end(), value, compare) - range.begin());
}

template <class Range, class T, class Compare = std::less<>>
std::size_t upper_bound_index(const Range& range, const T& value, Compare compare = {}) {
    return static_cast<std::size_t>(std::upper_bound(range.begin(), range.end(), value, compare) - range.begin());
}

template <class T>
std::vector<T> sorted_unique(std::vector<T> values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
    return values;
}

template <class T>
std::vector<std::size_t> coordinate_compress(const std::vector<T>& values) {
    const auto keys = sorted_unique(values);
    std::vector<std::size_t> result;
    result.reserve(values.size());
    for (const T& value : values) {
        result.push_back(lower_bound_index(keys, value));
    }
    return result;
}

// Transpose a rectangular matrix while preserving its row type.  For example,
// this turns both vector<vector<int>> and vector<string> into the same kind of
// matrix with rows and columns exchanged.
template <class Row>
std::vector<Row> transpose(const std::vector<Row>& matrix) {
    if (matrix.empty()) {
        return {};
    }

    const std::size_t row_count = matrix.size();
    const std::size_t column_count = matrix.front().size();
    for (const Row& row : matrix) {
        if (row.size() != column_count) {
            throw std::invalid_argument("transpose requires a rectangular matrix");
        }
    }

    using Cell = typename Row::value_type;
    std::vector<Row> result(column_count, Row(row_count, Cell{}));
    for (std::size_t row = 0; row < row_count; ++row) {
        for (std::size_t column = 0; column < column_count; ++column) {
            result[column][row] = matrix[row][column];
        }
    }
    return result;
}

// Rotate a rectangular matrix ninety degrees clockwise while preserving its row
// type. For example, this turns both vector<vector<int>> and vector<string>
// into the same kind of rotated matrix.
template <class Row>
std::vector<Row> rotate_clockwise(const std::vector<Row>& matrix) {
    std::vector<Row> result = transpose(matrix);
    for (Row& row : result) {
        std::reverse(row.begin(), row.end());
    }
    return result;
}

template <class Function>
class YCombinator {
public:
    explicit YCombinator(Function function) : function_(std::move(function)) {}

    template <class... Args>
    decltype(auto) operator()(Args&&... args) {
        return function_(std::ref(*this), std::forward<Args>(args)...);
    }

private:
    Function function_;
};

template <class Function>
YCombinator(Function) -> YCombinator<Function>;

struct SplitMix64Hash {
    static std::uint64_t mix(std::uint64_t value) {
        value += 0x9e3779b97f4a7c15ULL;
        value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
        return value ^ (value >> 31U);
    }

    std::size_t operator()(std::uint64_t value) const {
        static const std::uint64_t seed = static_cast<std::uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        return static_cast<std::size_t>(mix(value + seed));
    }
};

struct ZobristHash128 {
    std::uint64_t first = 0;
    std::uint64_t second = 0;

    constexpr auto operator<=>(const ZobristHash128&) const = default;
    constexpr ZobristHash128& operator^=(const ZobristHash128& other) {
        first ^= other.first;
        second ^= other.second;
        return *this;
    }
};

class Zobrist128 {
public:
    explicit Zobrist128(std::uint64_t seed) : seed_(seed) {}

    ZobristHash128 token(std::size_t key, std::uint8_t state) const {
        if (state == 0) {
            return {};
        }
        const std::uint64_t encoded = (static_cast<std::uint64_t>(key) << 8U) | state;
        return {SplitMix64Hash::mix(seed_ + encoded * 0x9e3779b97f4a7c15ULL),
                SplitMix64Hash::mix((seed_ ^ 0xd1b54a32d192ed03ULL) + encoded * 0xbf58476d1ce4e5b9ULL)};
    }

    void replace(ZobristHash128& hash, std::size_t key, std::uint8_t old_state, std::uint8_t new_state) const {
        hash ^= token(key, old_state);
        hash ^= token(key, new_state);
    }

private:
    std::uint64_t seed_;
};

}  // namespace cp

