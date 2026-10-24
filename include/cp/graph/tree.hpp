#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <limits>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight = long long>
class LowestCommonAncestor {
public:
    LowestCommonAncestor(const Graph<Weight>& tree, int root = 0)
        : depth_(static_cast<std::size_t>(tree.size())), distance_(static_cast<std::size_t>(tree.size())) {
        assert(tree.size() > 0 && 0 <= root && root < tree.size());
        const std::size_t levels = std::bit_width(static_cast<unsigned>(tree.size()));
        up_.assign(levels, std::vector<int>(static_cast<std::size_t>(tree.size()), root));
        build(tree, root, root);
        for (std::size_t level = 1; level < up_.size(); ++level) {
            for (int vertex = 0; vertex < tree.size(); ++vertex) {
                up_[level][vertex] = up_[level - 1][up_[level - 1][vertex]];
            }
        }
    }

    int lca(int lhs, int rhs) const {
        if (depth_[lhs] < depth_[rhs]) {
            std::swap(lhs, rhs);
        }
        lhs = kth_ancestor(lhs, depth_[lhs] - depth_[rhs]);
        if (lhs == rhs) {
            return lhs;
        }
        for (std::size_t level = up_.size(); level-- > 0;) {
            if (up_[level][lhs] != up_[level][rhs]) {
                lhs = up_[level][lhs];
                rhs = up_[level][rhs];
            }
        }
        return up_[0][lhs];
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

    int edge_distance(int lhs, int rhs) const {
        const int ancestor = lca(lhs, rhs);
        return depth_[lhs] + depth_[rhs] - 2 * depth_[ancestor];
    }

    Weight weighted_distance(int lhs, int rhs) const {
        const int ancestor = lca(lhs, rhs);
        return distance_[lhs] + distance_[rhs] - Weight{2} * distance_[ancestor];
    }

private:
    std::vector<std::vector<int>> up_;
    std::vector<int> depth_;
    std::vector<Weight> distance_;

    void build(const Graph<Weight>& tree, int vertex, int parent) {
        up_[0][vertex] = parent;
        for (const Edge<Weight>& edge : tree[vertex]) {
            if (edge.to == parent) {
                continue;
            }
            depth_[edge.to] = depth_[vertex] + 1;
            distance_[edge.to] = distance_[vertex] + edge.weight;
            build(tree, edge.to, vertex);
        }
    }
};

template <class Weight>
std::pair<Weight, std::vector<int>> tree_diameter(const Graph<Weight>& tree) {
    if (tree.size() == 0) {
        return {Weight{}, {}};
    }
    const auto farthest = [&](int source) {
        std::vector<Weight> distance(static_cast<std::size_t>(tree.size()), std::numeric_limits<Weight>::lowest());
        std::vector<int> parent(static_cast<std::size_t>(tree.size()), -1);
        const auto dfs = [&](auto&& self, int vertex, int previous) -> void {
            for (const Edge<Weight>& edge : tree[vertex]) {
                if (edge.to == previous) {
                    continue;
                }
                parent[edge.to] = vertex;
                distance[edge.to] = distance[vertex] + edge.weight;
                self(self, edge.to, vertex);
            }
        };
        distance[source] = Weight{};
        parent[source] = source;
        dfs(dfs, source, -1);
        const int endpoint = static_cast<int>(std::max_element(distance.begin(), distance.end()) - distance.begin());
        return std::tuple{endpoint, std::move(distance), std::move(parent)};
    };
    const int first = std::get<0>(farthest(0));
    const auto [second, distance, parent] = farthest(first);
    std::vector<int> path;
    for (int vertex = second;; vertex = parent[vertex]) {
        path.push_back(vertex);
        if (vertex == first) {
            break;
        }
    }
    std::reverse(path.begin(), path.end());
    return {distance[second], std::move(path)};
}

template <class Weight>
std::vector<int> tree_centers(const Graph<Weight>& tree) {
    if (tree.size() == 0) {
        return {};
    }
    std::vector<int> degree(static_cast<std::size_t>(tree.size()));
    std::queue<int> leaves;
    for (int vertex = 0; vertex < tree.size(); ++vertex) {
        degree[vertex] = static_cast<int>(tree[vertex].size());
        if (degree[vertex] <= 1) {
            leaves.push(vertex);
        }
    }
    int remaining = tree.size();
    while (remaining > 2) {
        const int layer_size = static_cast<int>(leaves.size());
        remaining -= layer_size;
        for (int count = 0; count < layer_size; ++count) {
            const int leaf = leaves.front();
            leaves.pop();
            for (const Edge<Weight>& edge : tree[leaf]) {
                if (--degree[edge.to] == 1) {
                    leaves.push(edge.to);
                }
            }
        }
    }
    std::vector<int> centers;
    while (!leaves.empty()) {
        centers.push_back(leaves.front());
        leaves.pop();
    }
    return centers;
}

}  // namespace cp

