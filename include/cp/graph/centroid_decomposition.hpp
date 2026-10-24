#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight = long long>
class CentroidDecomposition {
public:
    explicit CentroidDecomposition(const Graph<Weight>& tree)
        : n_(tree.size()),
          parent_(static_cast<std::size_t>(n_), -1),
          level_(static_cast<std::size_t>(n_), 0),
          children_(static_cast<std::size_t>(n_)),
          removed_(static_cast<std::size_t>(n_), false),
          subtree_size_(static_cast<std::size_t>(n_), 0) {
        assert(n_ > 0);
        root_ = decompose(tree, 0, -1, 0);
    }

    int root() const { return root_; }
    int size() const { return n_; }
    int parent(int vertex) const { return parent_[vertex]; }
    int level(int vertex) const { return level_[vertex]; }
    const std::vector<int>& children(int vertex) const { return children_[vertex]; }

    /**
     * Traverses vertex and all its ancestors in the centroid tree from vertex up to root.
     * Calls callback(int centroid).
     */
    template <class Function>
    void for_each_ancestor(int vertex, Function&& callback) const {
        for (int cur = vertex; cur != -1; cur = parent_[cur]) {
            callback(cur);
        }
    }

private:
    int n_;
    int root_ = 0;
    std::vector<int> parent_;
    std::vector<int> level_;
    std::vector<std::vector<int>> children_;
    std::vector<bool> removed_;
    std::vector<int> subtree_size_;

    void get_subtree_sizes(const Graph<Weight>& tree, int vertex, int parent) {
        subtree_size_[vertex] = 1;
        for (const Edge<Weight>& edge : tree[vertex]) {
            if (edge.to != parent && !removed_[edge.to]) {
                get_subtree_sizes(tree, edge.to, vertex);
                subtree_size_[vertex] += subtree_size_[edge.to];
            }
        }
    }

    int get_centroid(const Graph<Weight>& tree, int vertex, int parent, int total_size) {
        for (const Edge<Weight>& edge : tree[vertex]) {
            if (edge.to != parent && !removed_[edge.to]) {
                if (subtree_size_[edge.to] > total_size / 2) {
                    return get_centroid(tree, edge.to, vertex, total_size);
                }
            }
        }
        return vertex;
    }

    int decompose(const Graph<Weight>& tree, int start_vertex, int centroid_parent, int cur_level) {
        get_subtree_sizes(tree, start_vertex, -1);
        const int total_size = subtree_size_[start_vertex];
        const int centroid = get_centroid(tree, start_vertex, -1, total_size);

        removed_[centroid] = true;
        parent_[centroid] = centroid_parent;
        level_[centroid] = cur_level;

        for (const Edge<Weight>& edge : tree[centroid]) {
            if (!removed_[edge.to]) {
                const int child_centroid = decompose(tree, edge.to, centroid, cur_level + 1);
                children_[centroid].push_back(child_centroid);
            }
        }
        return centroid;
    }
};

}  // namespace cp
