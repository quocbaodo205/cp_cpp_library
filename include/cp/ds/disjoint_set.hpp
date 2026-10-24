#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

namespace cp {

class DisjointSet {
public:
    explicit DisjointSet(int n) : components_(n) {
        assert(n >= 0);
        parent_.resize(static_cast<std::size_t>(n));
        size_.assign(static_cast<std::size_t>(n), 1);
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    int leader(int vertex) {
        assert(0 <= vertex && vertex < static_cast<int>(parent_.size()));
        if (parent_[vertex] == vertex) {
            return vertex;
        }
        return parent_[vertex] = leader(parent_[vertex]);
    }

    bool merge(int lhs, int rhs) {
        lhs = leader(lhs);
        rhs = leader(rhs);
        if (lhs == rhs) {
            return false;
        }
        if (size_[lhs] < size_[rhs]) {
            std::swap(lhs, rhs);
        }
        parent_[rhs] = lhs;
        size_[lhs] += size_[rhs];
        --components_;
        return true;
    }

    bool same(int lhs, int rhs) {
        return leader(lhs) == leader(rhs);
    }

    int size(int vertex) {
        return size_[leader(vertex)];
    }

    int components() const { return components_; }

    std::vector<std::vector<int>> groups() {
        std::vector<std::vector<int>> result(parent_.size());
        for (int vertex = 0; vertex < static_cast<int>(parent_.size()); ++vertex) {
            result[leader(vertex)].push_back(vertex);
        }
        result.erase(std::remove_if(result.begin(), result.end(), [](const auto& group) { return group.empty(); }),
                     result.end());
        return result;
    }

private:
    std::vector<int> parent_;
    std::vector<int> size_;
    int components_;
};

}  // namespace cp

