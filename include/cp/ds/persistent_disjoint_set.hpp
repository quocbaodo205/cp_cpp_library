#pragma once

#include <cassert>
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

#include "cp/ds/persistent_array.hpp"

namespace cp {

class PersistentDisjointSet {
public:
    explicit PersistentDisjointSet(int n) : components_(n) {
        assert(n >= 0);
        std::vector<int> parent(static_cast<std::size_t>(n));
        std::iota(parent.begin(), parent.end(), 0);
        parent_ = PersistentArray<int>(std::move(parent));
        size_ = PersistentArray<int>(std::vector<int>(static_cast<std::size_t>(n), 1));
    }

    int leader(int vertex) const {
        assert(0 <= vertex && vertex < static_cast<int>(parent_.size()));
        while (parent_.get(vertex) != vertex) {
            vertex = parent_.get(vertex);
        }
        return vertex;
    }

    bool same(int lhs, int rhs) const { return leader(lhs) == leader(rhs); }
    int size(int vertex) const { return size_.get(leader(vertex)); }
    int components() const { return components_; }

    [[nodiscard]] PersistentDisjointSet merge(int lhs, int rhs) const {
        lhs = leader(lhs);
        rhs = leader(rhs);
        if (lhs == rhs) {
            return *this;
        }
        int lhs_size = size_.get(lhs);
        int rhs_size = size_.get(rhs);
        if (lhs_size < rhs_size) {
            std::swap(lhs, rhs);
            std::swap(lhs_size, rhs_size);
        }
        return PersistentDisjointSet(parent_.set(rhs, lhs),
                                     size_.set(lhs, lhs_size + rhs_size),
                                     components_ - 1);
    }

private:
    PersistentArray<int> parent_;
    PersistentArray<int> size_;
    int components_ = 0;

    PersistentDisjointSet(PersistentArray<int> parent, PersistentArray<int> size, int components)
        : parent_(std::move(parent)), size_(std::move(size)), components_(components) {}
};

}  // namespace cp

