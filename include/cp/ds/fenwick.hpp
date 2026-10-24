#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

namespace cp {

template <class Monoid>
class MonoidFenwickTree {
public:
    using T = typename Monoid::value_type;

    explicit MonoidFenwickTree(std::size_t size) : bit_(size + 1, Monoid::identity()) {}

    std::size_t size() const { return bit_.size() - 1; }

    void update(std::size_t index, const T& value) {
        assert(index < size());
        for (++index; index < bit_.size(); index += index & (~index + 1)) {
            bit_[index] = Monoid::combine(bit_[index], value);
        }
    }

    T prefix_fold(std::size_t end) const {
        assert(end <= size());
        T result = Monoid::identity();
        for (; end > 0; end -= end & (~end + 1)) {
            result = Monoid::combine(result, bit_[end]);
        }
        return result;
    }

private:
    std::vector<T> bit_;
};

template <class T>
class FenwickTree {
public:
    FenwickTree() = default;
    explicit FenwickTree(std::size_t size) : bit_(size + 1, T{}) {}

    std::size_t size() const { return bit_.empty() ? 0 : bit_.size() - 1; }

    void add(std::size_t index, const T& delta) {
        assert(index < size());
        for (++index; index < bit_.size(); index += index & (~index + 1)) {
            bit_[index] += delta;
        }
    }

    T prefix_sum(std::size_t end) const {
        assert(end <= size());
        T result{};
        for (; end > 0; end -= end & (~end + 1)) {
            result += bit_[end];
        }
        return result;
    }

    T sum(std::size_t left, std::size_t right) const {
        assert(left <= right && right <= size());
        return prefix_sum(right) - prefix_sum(left);
    }

    std::size_t lower_bound(const T& target) const {
        if (target <= T{}) {
            return 0;
        }
        std::size_t index = 0;
        T accumulated{};
        for (std::size_t step = std::bit_floor(size()); step > 0; step >>= 1U) {
            const std::size_t next = index + step;
            if (next <= size() && accumulated + bit_[next] < target) {
                index = next;
                accumulated += bit_[next];
            }
        }
        return index < size() ? index : size();
    }

private:
    std::vector<T> bit_;
};

class OrderedMultiset {
public:
    explicit OrderedMultiset(std::size_t value_count) : frequencies_(value_count) {}

    void insert(std::size_t value, int count = 1) { frequencies_.add(value, count); }
    void erase(std::size_t value, int count = 1) { frequencies_.add(value, -count); }
    int order_of_key(std::size_t value) const { return frequencies_.prefix_sum(value); }
    std::size_t find_by_order(std::size_t order) const {
        return frequencies_.lower_bound(static_cast<int>(order + 1));
    }
    int size() const { return frequencies_.prefix_sum(frequencies_.size()); }

private:
    FenwickTree<int> frequencies_;
};

}  // namespace cp

