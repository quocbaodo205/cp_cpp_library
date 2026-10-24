#pragma once

#include <cstddef>
#include <functional>
#include <span>
#include <vector>

namespace cp {

struct CartesianTree {
    std::vector<int> parent;
    std::vector<int> left;
    std::vector<int> right;
    int root = -1;
};

template <class T, class Compare = std::less<T>>
CartesianTree make_cartesian_tree(std::span<const T> values, Compare compare = {}) {
    const int n = static_cast<int>(values.size());
    CartesianTree tree{std::vector<int>(values.size(), -1), std::vector<int>(values.size(), -1),
                       std::vector<int>(values.size(), -1), -1};
    std::vector<int> stack;
    stack.reserve(values.size());
    for (int index = 0; index < n; ++index) {
        int last = -1;
        while (!stack.empty() && compare(values[index], values[stack.back()])) {
            last = stack.back();
            stack.pop_back();
        }
        if (!stack.empty()) {
            tree.parent[index] = stack.back();
            tree.right[stack.back()] = index;
        }
        if (last != -1) {
            tree.parent[last] = index;
            tree.left[index] = last;
        }
        stack.push_back(index);
    }
    if (!stack.empty()) {
        tree.root = stack.front();
    }
    return tree;
}

}  // namespace cp

