#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight = long long>
class RootedTree {
public:
    explicit RootedTree(const Graph<Weight>& tree, int root = 0)
        : root_(root),
          n_(tree.size()),
          parent_(static_cast<std::size_t>(n_), -1),
          depth_(static_cast<std::size_t>(n_), 0),
          subtree_size_(static_cast<std::size_t>(n_), 1),
          distance_(static_cast<std::size_t>(n_), Weight{}),
          time_in_(static_cast<std::size_t>(n_), 0),
          time_out_(static_cast<std::size_t>(n_), 0),
          children_(static_cast<std::size_t>(n_)) {
        assert(n_ > 0 && 0 <= root && root < n_);
        const std::size_t levels = std::bit_width(static_cast<unsigned>(n_));
        up_.assign(levels, std::vector<int>(static_cast<std::size_t>(n_), root));
        order_.reserve(static_cast<std::size_t>(n_));

        int timer = 0;
        build(tree, root, root, timer);
        for (std::size_t level = 1; level < up_.size(); ++level) {
            for (int vertex = 0; vertex < n_; ++vertex) {
                up_[level][vertex] = up_[level - 1][up_[level - 1][vertex]];
            }
        }
    }

    int root() const { return root_; }
    int size() const { return n_; }
    int parent(int vertex) const { return parent_[vertex]; }
    int depth(int vertex) const { return depth_[vertex]; }
    int subtree_size(int vertex) const { return subtree_size_[vertex]; }
    Weight distance(int vertex) const { return distance_[vertex]; }
    int time_in(int vertex) const { return time_in_[vertex]; }
    int time_out(int vertex) const { return time_out_[vertex]; }
    const std::vector<int>& children(int vertex) const { return children_[vertex]; }
    const std::vector<int>& order() const { return order_; }

    std::pair<int, int> subtree_interval(int vertex) const {
        return {time_in_[vertex], time_out_[vertex]};
    }

    bool is_ancestor(int ancestor, int descendant) const {
        return time_in_[ancestor] <= time_in_[descendant] && time_out_[descendant] <= time_out_[ancestor];
    }

    int kth_ancestor(int vertex, int steps) const {
        assert(steps >= 0);
        for (std::size_t bit = 0; bit < up_.size(); ++bit) {
            if (((static_cast<unsigned>(steps) >> bit) & 1U) != 0) {
                vertex = up_[bit][vertex];
            }
        }
        return vertex;
    }

    int lca(int lhs, int rhs) const {
        if (is_ancestor(lhs, rhs)) {
            return lhs;
        }
        if (is_ancestor(rhs, lhs)) {
            return rhs;
        }
        for (std::size_t level = up_.size(); level-- > 0;) {
            if (!is_ancestor(up_[level][lhs], rhs)) {
                lhs = up_[level][lhs];
            }
        }
        return up_[0][lhs];
    }

    int edge_distance(int lhs, int rhs) const {
        const int ancestor = lca(lhs, rhs);
        return depth_[lhs] + depth_[rhs] - 2 * depth_[ancestor];
    }

    Weight weighted_distance(int lhs, int rhs) const {
        const int ancestor = lca(lhs, rhs);
        return distance_[lhs] + distance_[rhs] - Weight{2} * distance_[ancestor];
    }

    std::vector<int> path(int from, int to) const {
        const int ancestor = lca(from, to);
        std::vector<int> up_path;
        for (int cur = from; cur != ancestor; cur = parent_[cur]) {
            up_path.push_back(cur);
        }
        up_path.push_back(ancestor);
        std::vector<int> down_path;
        for (int cur = to; cur != ancestor; cur = parent_[cur]) {
            down_path.push_back(cur);
        }
        std::reverse(down_path.begin(), down_path.end());
        up_path.insert(up_path.end(), down_path.begin(), down_path.end());
        return up_path;
    }

private:
    int root_;
    int n_;
    std::vector<int> parent_;
    std::vector<int> depth_;
    std::vector<int> subtree_size_;
    std::vector<Weight> distance_;
    std::vector<int> time_in_;
    std::vector<int> time_out_;
    std::vector<int> order_;
    std::vector<std::vector<int>> children_;
    std::vector<std::vector<int>> up_;

    void build(const Graph<Weight>& tree, int vertex, int parent, int& timer) {
        parent_[vertex] = (vertex == root_ ? -1 : parent);
        up_[0][vertex] = parent;
        time_in_[vertex] = timer++;
        order_.push_back(vertex);

        for (const Edge<Weight>& edge : tree[vertex]) {
            if (edge.to == parent) {
                continue;
            }
            depth_[edge.to] = depth_[vertex] + 1;
            distance_[edge.to] = distance_[vertex] + edge.weight;
            children_[vertex].push_back(edge.to);
            build(tree, edge.to, vertex, timer);
            subtree_size_[vertex] += subtree_size_[edge.to];
        }
        time_out_[vertex] = timer;
    }
};

/**
 * DSU on tree (Sack / small-to-large merging technique).
 *
 * Calls:
 * - add(vertex): adds vertex to current structure
 * - remove(vertex): removes vertex from current structure
 * - query(vertex): evaluates result for vertex when its entire subtree is in structure
 */
template <class Weight, class Add, class Remove, class Query>
void dsu_on_tree(const RootedTree<Weight>& tree, Add&& add, Remove&& remove, Query&& query) {
    const auto dfs = [&](auto&& self, int vertex, bool keep) -> void {
        int heavy = -1;
        int max_size = -1;
        for (int child : tree.children(vertex)) {
            if (tree.subtree_size(child) > max_size) {
                max_size = tree.subtree_size(child);
                heavy = child;
            }
        }

        for (int child : tree.children(vertex)) {
            if (child != heavy) {
                self(self, child, false);
            }
        }
        if (heavy != -1) {
            self(self, heavy, true);
        }

        const auto modify_subtree = [&](int sub_root, auto&& op) {
            const auto [left, right] = tree.subtree_interval(sub_root);
            for (int idx = left; idx < right; ++idx) {
                op(tree.order()[idx]);
            }
        };

        for (int child : tree.children(vertex)) {
            if (child != heavy) {
                modify_subtree(child, add);
            }
        }
        add(vertex);
        query(vertex);

        if (!keep) {
            modify_subtree(vertex, remove);
        }
    };

    dfs(dfs, tree.root(), true);
}

}  // namespace cp
