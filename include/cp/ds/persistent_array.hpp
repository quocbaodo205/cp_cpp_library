#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace cp {

template <class T>
class PersistentArray {
    struct Node {
        T value{};
        std::shared_ptr<const Node> left;
        std::shared_ptr<const Node> right;
    };

public:
    PersistentArray() = default;

    explicit PersistentArray(std::vector<T> values) : size_(values.size()), root_(build(values, 0, values.size())) {}

    std::size_t size() const { return size_; }

    const T& get(std::size_t index) const {
        assert(index < size_);
        return get(root_, 0, size_, index);
    }

    [[nodiscard]] PersistentArray set(std::size_t index, T value) const {
        assert(index < size_);
        return PersistentArray(size_, set(root_, 0, size_, index, std::move(value)));
    }

private:
    std::size_t size_ = 0;
    std::shared_ptr<const Node> root_;

    PersistentArray(std::size_t size, std::shared_ptr<const Node> root) : size_(size), root_(std::move(root)) {}

    static std::shared_ptr<const Node> build(const std::vector<T>& values, std::size_t left, std::size_t right) {
        if (left == right) {
            return nullptr;
        }
        if (right - left == 1) {
            return std::make_shared<Node>(Node{values[left], nullptr, nullptr});
        }
        const std::size_t middle = left + (right - left) / 2;
        return std::make_shared<Node>(Node{T{}, build(values, left, middle), build(values, middle, right)});
    }

    static const T& get(const std::shared_ptr<const Node>& node, std::size_t left, std::size_t right,
                        std::size_t index) {
        if (right - left == 1) {
            return node->value;
        }
        const std::size_t middle = left + (right - left) / 2;
        return index < middle ? get(node->left, left, middle, index) : get(node->right, middle, right, index);
    }

    static std::shared_ptr<const Node> set(const std::shared_ptr<const Node>& node, std::size_t left,
                                           std::size_t right, std::size_t index, T value) {
        if (right - left == 1) {
            return std::make_shared<Node>(Node{std::move(value), nullptr, nullptr});
        }
        const std::size_t middle = left + (right - left) / 2;
        if (index < middle) {
            return std::make_shared<Node>(Node{T{}, set(node->left, left, middle, index, std::move(value)), node->right});
        }
        return std::make_shared<Node>(Node{T{}, node->left, set(node->right, middle, right, index, std::move(value))});
    }
};

}  // namespace cp

