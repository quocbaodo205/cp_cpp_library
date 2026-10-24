#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

namespace cp {

template <class Monoid>
class SparseTable {
public:
    using T = typename Monoid::value_type;

    explicit SparseTable(const std::vector<T>& values) {
        if (values.empty()) {
            return;
        }
        const std::size_t levels = std::bit_width(values.size());
        table_.assign(levels, values);
        for (std::size_t level = 1; level < levels; ++level) {
            const std::size_t width = std::size_t{1} << level;
            for (std::size_t left = 0; left + width <= values.size(); ++left) {
                table_[level][left] = Monoid::combine(table_[level - 1][left],
                                                     table_[level - 1][left + width / 2]);
            }
        }
    }

    // Monoid must be idempotent: min, max, gcd, bit-and, or bit-or.
    T fold(std::size_t left, std::size_t right) const {
        assert(left < right && !table_.empty() && right <= table_[0].size());
        const std::size_t level = std::bit_width(right - left) - 1;
        return Monoid::combine(table_[level][left], table_[level][right - (std::size_t{1} << level)]);
    }

private:
    std::vector<std::vector<T>> table_;
};

}  // namespace cp

