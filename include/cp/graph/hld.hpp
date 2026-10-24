#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight = long long>
class HeavyLightDecomposition {
public:
    explicit HeavyLightDecomposition(const Graph<Weight>& tree, int root = 0)
        : root_(root),
          n_(tree.size()),
          parent_(static_cast<std::size_t>(n_), -1),
          depth_(static_cast<std::size_t>(n_), 0),
          subtree_size_(static_cast<std::size_t>(n_), 1),
          heavy_child_(static_cast<std::size_t>(n_), -1),
          head_(static_cast<std::size_t>(n_), root),
          in_(static_cast<std::size_t>(n_), 0),
          out_(static_cast<std::size_t>(n_), 0),
          order_(static_cast<std::size_t>(n_), 0) {
        assert(n_ > 0 && 0 <= root && root < n_);
        dfs_size(tree, root, -1, 0);
        int timer = 0;
        dfs_hld(tree, root, root, timer);
    }

    int size() const { return n_; }
    int root() const { return root_; }
    int parent(int vertex) const { return parent_[vertex]; }
    int depth(int vertex) const { return depth_[vertex]; }
    int subtree_size(int vertex) const { return subtree_size_[vertex]; }
    int heavy_child(int vertex) const { return heavy_child_[vertex]; }
    int head(int vertex) const { return head_[vertex]; }
    int in(int vertex) const { return in_[vertex]; }
    int out(int vertex) const { return out_[vertex]; }
    int vertex_at(int in_idx) const { return order_[in_idx]; }

    std::pair<int, int> subtree_interval(int vertex) const {
        return {in_[vertex], out_[vertex]};
    }

    std::pair<int, int> vertex_interval(int vertex) const {
        return {in_[vertex], in_[vertex] + 1};
    }

    int lca(int lhs, int rhs) const {
        while (head_[lhs] != head_[rhs]) {
            if (depth_[head_[lhs]] > depth_[head_[rhs]]) {
                lhs = parent_[head_[lhs]];
            } else {
                rhs = parent_[head_[rhs]];
            }
        }
        return depth_[lhs] < depth_[rhs] ? lhs : rhs;
    }

    int dist(int lhs, int rhs) const {
        return depth_[lhs] + depth_[rhs] - 2 * depth_[lca(lhs, rhs)];
    }

    int kth_ancestor(int vertex, int steps) const {
        assert(steps >= 0);
        if (depth_[vertex] < steps) {
            return -1;
        }
        while (vertex != -1) {
            const int chain_head = head_[vertex];
            const int chain_length = depth_[vertex] - depth_[chain_head];
            if (steps <= chain_length) {
                return order_[in_[vertex] - steps];
            }
            steps -= (chain_length + 1);
            vertex = parent_[chain_head];
        }
        return -1;
    }

    /**
     * Iterates over half-open [left, right) intervals in the decomposition order
     * that cover all vertices on the path between lhs and rhs.
     * Order of intervals is not necessarily from lhs to rhs.
     */
    template <class Function>
    void for_each_path_interval(int lhs, int rhs, Function&& func) const {
        while (head_[lhs] != head_[rhs]) {
            if (depth_[head_[lhs]] > depth_[head_[rhs]]) {
                func(in_[head_[lhs]], in_[lhs] + 1);
                lhs = parent_[head_[lhs]];
            } else {
                func(in_[head_[rhs]], in_[rhs] + 1);
                rhs = parent_[head_[rhs]];
            }
        }
        const int left = std::min(in_[lhs], in_[rhs]);
        const int right = std::max(in_[lhs], in_[rhs]);
        func(left, right + 1);
    }

    /**
     * Iterates over half-open [left, right) intervals covering all edges on the path
     * between lhs and rhs (the LCA vertex is omitted).
     */
    template <class Function>
    void for_each_edge_path_interval(int lhs, int rhs, Function&& func) const {
        while (head_[lhs] != head_[rhs]) {
            if (depth_[head_[lhs]] > depth_[head_[rhs]]) {
                func(in_[head_[lhs]], in_[lhs] + 1);
                lhs = parent_[head_[lhs]];
            } else {
                func(in_[head_[rhs]], in_[rhs] + 1);
                rhs = parent_[head_[rhs]];
            }
        }
        if (lhs != rhs) {
            const int left = std::min(in_[lhs], in_[rhs]);
            const int right = std::max(in_[lhs], in_[rhs]);
            func(left + 1, right + 1);
        }
    }

    /**
     * Iterates over directed path from lhs to rhs.
     * Passes (int left, int right, bool is_up) to func, where:
     * - is_up is true if the step goes from child towards ancestor [left, right)
     * - is_up is false if the step goes from ancestor towards child [left, right)
     */
    template <class Function>
    void for_each_directed_path_interval(int lhs, int rhs, Function&& func) const {
        std::vector<std::pair<int, int>> up_intervals;
        std::vector<std::pair<int, int>> down_intervals;

        while (head_[lhs] != head_[rhs]) {
            if (depth_[head_[lhs]] > depth_[head_[rhs]]) {
                up_intervals.emplace_back(in_[head_[lhs]], in_[lhs] + 1);
                lhs = parent_[head_[lhs]];
            } else {
                down_intervals.emplace_back(in_[head_[rhs]], in_[rhs] + 1);
                rhs = parent_[head_[rhs]];
            }
        }
        if (depth_[lhs] >= depth_[rhs]) {
            up_intervals.emplace_back(in_[rhs], in_[lhs] + 1);
        } else {
            down_intervals.emplace_back(in_[lhs], in_[rhs] + 1);
        }

        for (const auto& [left, right] : up_intervals) {
            func(left, right, true);
        }
        std::reverse(down_intervals.begin(), down_intervals.end());
        for (const auto& [left, right] : down_intervals) {
            func(left, right, false);
        }
    }

private:
    int root_;
    int n_;
    std::vector<int> parent_;
    std::vector<int> depth_;
    std::vector<int> subtree_size_;
    std::vector<int> heavy_child_;
    std::vector<int> head_;
    std::vector<int> in_;
    std::vector<int> out_;
    std::vector<int> order_;

    void dfs_size(const Graph<Weight>& tree, int vertex, int parent, int depth) {
        parent_[vertex] = parent;
        depth_[vertex] = depth;
        subtree_size_[vertex] = 1;
        int max_child_size = 0;

        for (const Edge<Weight>& edge : tree[vertex]) {
            if (edge.to == parent) {
                continue;
            }
            dfs_size(tree, edge.to, vertex, depth + 1);
            subtree_size_[vertex] += subtree_size_[edge.to];
            if (subtree_size_[edge.to] > max_child_size) {
                max_child_size = subtree_size_[edge.to];
                heavy_child_[vertex] = edge.to;
            }
        }
    }

    void dfs_hld(const Graph<Weight>& tree, int vertex, int head, int& timer) {
        head_[vertex] = head;
        in_[vertex] = timer;
        order_[timer] = vertex;
        ++timer;

        if (heavy_child_[vertex] != -1) {
            dfs_hld(tree, heavy_child_[vertex], head, timer);
        }

        for (const Edge<Weight>& edge : tree[vertex]) {
            if (edge.to == parent_[vertex] || edge.to == heavy_child_[vertex]) {
                continue;
            }
            dfs_hld(tree, edge.to, edge.to, timer);
        }
        out_[vertex] = timer;
    }
};

}  // namespace cp
