#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

namespace cp {

template <class Monoid>
class SegmentTree {
public:
    using T = typename Monoid::value_type;

    SegmentTree() = default;
    explicit SegmentTree(std::size_t n)
        : n_(n), size_(std::bit_ceil(std::max<std::size_t>(n, 1))), data_(size_ * 2, Monoid::identity()) {}

    explicit SegmentTree(const std::vector<T>& values) : SegmentTree(values.size()) {
        for (std::size_t i = 0; i < n_; ++i) {
            data_[size_ + i] = values[i];
        }
        for (std::size_t i = size_; i-- > 1;) {
            pull(i);
        }
    }

    std::size_t size() const { return n_; }

    void set(std::size_t index, const T& value) {
        assert(index < n_);
        index += size_;
        data_[index] = value;
        while ((index >>= 1U) > 0) {
            pull(index);
        }
    }

    const T& get(std::size_t index) const {
        assert(index < n_);
        return data_[size_ + index];
    }

    T fold(std::size_t left, std::size_t right) const {
        assert(left <= right && right <= n_);
        T left_result = Monoid::identity();
        T right_result = Monoid::identity();
        for (left += size_, right += size_; left < right; left >>= 1U, right >>= 1U) {
            if ((left & 1U) != 0) {
                left_result = Monoid::combine(left_result, data_[left++]);
            }
            if ((right & 1U) != 0) {
                right_result = Monoid::combine(data_[--right], right_result);
            }
        }
        return Monoid::combine(left_result, right_result);
    }

    T all_fold() const { return data_[1]; }

    template <class Predicate>
    std::size_t max_right(std::size_t left, Predicate predicate) const {
        assert(left <= n_ && predicate(Monoid::identity()));
        if (left == n_) {
            return n_;
        }
        left += size_;
        T accumulated = Monoid::identity();
        do {
            while ((left & 1U) == 0) {
                left >>= 1U;
            }
            const T candidate = Monoid::combine(accumulated, data_[left]);
            if (!predicate(candidate)) {
                while (left < size_) {
                    left <<= 1U;
                    const T next = Monoid::combine(accumulated, data_[left]);
                    if (predicate(next)) {
                        accumulated = next;
                        ++left;
                    }
                }
                return std::min(left - size_, n_);
            }
            accumulated = candidate;
            ++left;
        } while (!std::has_single_bit(left));
        return n_;
    }

    template <class Predicate>
    std::size_t min_left(std::size_t right, Predicate predicate) const {
        assert(right <= n_ && predicate(Monoid::identity()));
        if (right == 0) {
            return 0;
        }
        right += size_;
        T accumulated = Monoid::identity();
        do {
            --right;
            while (right > 1 && (right & 1U) != 0) {
                right >>= 1U;
            }
            const T candidate = Monoid::combine(data_[right], accumulated);
            if (!predicate(candidate)) {
                while (right < size_) {
                    right = right * 2 + 1;
                    const T next = Monoid::combine(data_[right], accumulated);
                    if (predicate(next)) {
                        accumulated = next;
                        --right;
                    }
                }
                return std::min(right + 1 - size_, n_);
            }
            accumulated = candidate;
        } while (!std::has_single_bit(right));
        return 0;
    }

private:
    std::size_t n_ = 0;
    std::size_t size_ = 1;
    std::vector<T> data_{Monoid::identity(), Monoid::identity()};

    void pull(std::size_t index) {
        data_[index] = Monoid::combine(data_[index * 2], data_[index * 2 + 1]);
    }
};

}  // namespace cp

