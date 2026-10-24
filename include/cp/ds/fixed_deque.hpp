#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <utility>

namespace cp {

template <class T, std::size_t Capacity>
class FixedDeque {
    static_assert(Capacity > 0);

public:
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == Capacity; }
    std::size_t size() const { return size_; }

    T& front() {
        assert(!empty());
        return *data_[left_];
    }

    const T& front() const {
        assert(!empty());
        return *data_[left_];
    }

    T& back() {
        assert(!empty());
        return *data_[(left_ + size_ - 1) % Capacity];
    }

    const T& back() const {
        assert(!empty());
        return *data_[(left_ + size_ - 1) % Capacity];
    }

    void push_front(T value) {
        assert(!full());
        left_ = (left_ + Capacity - 1) % Capacity;
        data_[left_] = std::move(value);
        ++size_;
    }

    void push_back(T value) {
        assert(!full());
        data_[(left_ + size_) % Capacity] = std::move(value);
        ++size_;
    }

    T pop_front() {
        assert(!empty());
        T value = std::move(*data_[left_]);
        data_[left_].reset();
        left_ = (left_ + 1) % Capacity;
        --size_;
        return value;
    }

    T pop_back() {
        assert(!empty());
        const std::size_t index = (left_ + size_ - 1) % Capacity;
        T value = std::move(*data_[index]);
        data_[index].reset();
        --size_;
        return value;
    }

private:
    std::array<std::optional<T>, Capacity> data_{};
    std::size_t left_ = 0;
    std::size_t size_ = 0;
};

}  // namespace cp

