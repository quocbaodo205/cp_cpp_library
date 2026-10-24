#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <vector>

namespace cp {

// Action supplies value_type, operator_type, identity(), combine(),
// identity_operator(), apply(op, value, segment_length), and compose(newer, older).
template <class Action>
class LazySegmentTree {
public:
    using T = typename Action::value_type;
    using F = typename Action::operator_type;

    explicit LazySegmentTree(std::size_t n)
        : n_(n), size_(std::bit_ceil(std::max<std::size_t>(n, 1))), log_(std::bit_width(size_) - 1),
          data_(size_ * 2, Action::identity()), lazy_(size_, Action::identity_operator()), length_(size_ * 2, 1) {
        for (std::size_t i = size_; i-- > 1;) {
            length_[i] = length_[i * 2] + length_[i * 2 + 1];
        }
    }

    explicit LazySegmentTree(const std::vector<T>& values) : LazySegmentTree(values.size()) {
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
        const std::size_t leaf = index + size_;
        for (std::size_t level = log_; level > 0; --level) {
            push(leaf >> level);
        }
        data_[leaf] = value;
        for (std::size_t level = 1; level <= log_; ++level) {
            pull(leaf >> level);
        }
    }

    T get(std::size_t index) {
        assert(index < n_);
        index += size_;
        for (std::size_t level = log_; level > 0; --level) {
            push(index >> level);
        }
        return data_[index];
    }

    T fold(std::size_t left, std::size_t right) {
        assert(left <= right && right <= n_);
        if (left == right) {
            return Action::identity();
        }
        left += size_;
        right += size_;
        for (std::size_t level = log_; level > 0; --level) {
            if (((left >> level) << level) != left) {
                push(left >> level);
            }
            if (((right >> level) << level) != right) {
                push((right - 1) >> level);
            }
        }
        T left_result = Action::identity();
        T right_result = Action::identity();
        while (left < right) {
            if ((left & 1U) != 0) {
                left_result = Action::combine(left_result, data_[left++]);
            }
            if ((right & 1U) != 0) {
                right_result = Action::combine(data_[--right], right_result);
            }
            left >>= 1U;
            right >>= 1U;
        }
        return Action::combine(left_result, right_result);
    }

    void apply(std::size_t left, std::size_t right, const F& operation) {
        assert(left <= right && right <= n_);
        if (left == right) {
            return;
        }
        left += size_;
        right += size_;
        const std::size_t original_left = left;
        const std::size_t original_right = right;
        for (std::size_t level = log_; level > 0; --level) {
            if (((left >> level) << level) != left) {
                push(left >> level);
            }
            if (((right >> level) << level) != right) {
                push((right - 1) >> level);
            }
        }
        while (left < right) {
            if ((left & 1U) != 0) {
                apply_at(left++, operation);
            }
            if ((right & 1U) != 0) {
                apply_at(--right, operation);
            }
            left >>= 1U;
            right >>= 1U;
        }
        for (std::size_t level = 1; level <= log_; ++level) {
            if (((original_left >> level) << level) != original_left) {
                pull(original_left >> level);
            }
            if (((original_right >> level) << level) != original_right) {
                pull((original_right - 1) >> level);
            }
        }
    }

    T all_fold() const { return data_[1]; }

private:
    std::size_t n_;
    std::size_t size_;
    std::size_t log_;
    std::vector<T> data_;
    std::vector<F> lazy_;
    std::vector<std::size_t> length_;

    void pull(std::size_t index) {
        data_[index] = Action::combine(data_[index * 2], data_[index * 2 + 1]);
    }

    void apply_at(std::size_t index, const F& operation) {
        data_[index] = Action::apply(operation, data_[index], length_[index]);
        if (index < size_) {
            lazy_[index] = Action::compose(operation, lazy_[index]);
        }
    }

    void push(std::size_t index) {
        apply_at(index * 2, lazy_[index]);
        apply_at(index * 2 + 1, lazy_[index]);
        lazy_[index] = Action::identity_operator();
    }
};

template <class T>
struct RangeAddRangeSum {
    using value_type = T;
    using operator_type = T;
    static constexpr T identity() { return T{}; }
    static constexpr T combine(const T& left, const T& right) { return left + right; }
    static constexpr T identity_operator() { return T{}; }
    static constexpr T apply(const T& operation, const T& value, std::size_t length) {
        return value + operation * static_cast<T>(length);
    }
    static constexpr T compose(const T& newer, const T& older) { return newer + older; }
};

}  // namespace cp

