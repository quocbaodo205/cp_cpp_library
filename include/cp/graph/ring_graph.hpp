#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <queue>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight = long long>
class RingGraph {
public:
    explicit RingGraph(const Graph<Weight>& graph)
        : n_(graph.size()),
          is_on_cycle_(static_cast<std::size_t>(n_), false),
          cycle_index_(static_cast<std::size_t>(n_), -1),
          root_on_cycle_(static_cast<std::size_t>(n_), -1),
          distance_to_cycle_(static_cast<std::size_t>(n_), -1),
          tree_parent_(static_cast<std::size_t>(n_), -1) {
        if (n_ == 0) {
            return;
        }

        std::vector<int> degree(static_cast<std::size_t>(n_), 0);
        std::queue<int> leaves;
        for (int vertex = 0; vertex < n_; ++vertex) {
            degree[vertex] = static_cast<int>(graph[vertex].size());
            if (degree[vertex] <= 1) {
                leaves.push(vertex);
            }
        }

        std::vector<bool> in_tree(static_cast<std::size_t>(n_), false);
        while (!leaves.empty()) {
            const int leaf = leaves.front();
            leaves.pop();
            in_tree[leaf] = true;
            for (const Edge<Weight>& edge : graph[leaf]) {
                if (--degree[edge.to] == 1) {
                    leaves.push(edge.to);
                }
            }
        }

        // Find a cycle vertex
        int start_cycle = -1;
        for (int vertex = 0; vertex < n_; ++vertex) {
            if (!in_tree[vertex]) {
                start_cycle = vertex;
                break;
            }
        }

        if (start_cycle == -1) {
            // Forest without cycles
            return;
        }

        // Trace the cycle
        int current = start_cycle;
        int previous = -1;
        while (true) {
            cycle_.push_back(current);
            is_on_cycle_[current] = true;
            cycle_index_[current] = static_cast<int>(cycle_.size()) - 1;

            int next_vertex = -1;
            for (const Edge<Weight>& edge : graph[current]) {
                if (!in_tree[edge.to] && edge.to != previous) {
                    next_vertex = edge.to;
                    break;
                }
            }

            if (next_vertex == -1 || next_vertex == start_cycle) {
                break;
            }
            previous = current;
            current = next_vertex;
        }

        // Traverse trees attached to each cycle vertex
        for (int c_vertex : cycle_) {
            root_on_cycle_[c_vertex] = c_vertex;
            distance_to_cycle_[c_vertex] = 0;
            tree_parent_[c_vertex] = -1;

            std::queue<int> queue;
            queue.push(c_vertex);
            while (!queue.empty()) {
                const int cur = queue.front();
                queue.pop();
                for (const Edge<Weight>& edge : graph[cur]) {
                    if (in_tree[edge.to] && root_on_cycle_[edge.to] == -1) {
                        root_on_cycle_[edge.to] = c_vertex;
                        distance_to_cycle_[edge.to] = distance_to_cycle_[cur] + 1;
                        tree_parent_[edge.to] = cur;
                        queue.push(edge.to);
                    }
                }
            }
        }
    }

    int size() const { return n_; }
    const std::vector<int>& cycle() const { return cycle_; }
    int cycle_length() const { return static_cast<int>(cycle_.size()); }
    bool is_on_cycle(int vertex) const { return is_on_cycle_[vertex]; }
    int cycle_index(int vertex) const { return cycle_index_[vertex]; }
    int root_on_cycle(int vertex) const { return root_on_cycle_[vertex]; }
    int distance_to_cycle(int vertex) const { return distance_to_cycle_[vertex]; }
    int tree_parent(int vertex) const { return tree_parent_[vertex]; }

    /**
     * Shortest distance along cycle between two cycle vertices lhs and rhs.
     */
    int cycle_distance(int lhs, int rhs) const {
        assert(is_on_cycle(lhs) && is_on_cycle(rhs));
        const int i = cycle_index_[lhs];
        const int j = cycle_index_[rhs];
        const int k = cycle_length();
        const int diff = std::abs(i - j);
        return std::min(diff, k - diff);
    }

    /**
     * Shortest path along cycle from lhs to rhs.
     */
    std::vector<int> cycle_path(int lhs, int rhs) const {
        assert(is_on_cycle(lhs) && is_on_cycle(rhs));
        const int i = cycle_index_[lhs];
        const int j = cycle_index_[rhs];
        const int k = cycle_length();
        const int forward_dist = (j - i + k) % k;
        const int backward_dist = (i - j + k) % k;

        std::vector<int> path;
        if (forward_dist <= backward_dist) {
            for (int step = 0; step <= forward_dist; ++step) {
                path.push_back(cycle_[(i + step) % k]);
            }
        } else {
            for (int step = 0; step <= backward_dist; ++step) {
                path.push_back(cycle_[(i - step + k) % k]);
            }
        }
        return path;
    }

    /**
     * Shortest path on pseudotree between any two vertices lhs and rhs.
     */
    std::vector<int> path(int lhs, int rhs) const {
        const int r_lhs = root_on_cycle_[lhs];
        const int r_rhs = root_on_cycle_[rhs];
        assert(r_lhs != -1 && r_rhs != -1);

        if (r_lhs == r_rhs) {
            // Both are in the same tree rooted at r_lhs
            std::vector<int> path_lhs;
            for (int cur = lhs; cur != r_lhs; cur = tree_parent_[cur]) {
                path_lhs.push_back(cur);
            }
            path_lhs.push_back(r_lhs);

            std::vector<int> path_rhs;
            for (int cur = rhs; cur != r_rhs; cur = tree_parent_[cur]) {
                path_rhs.push_back(cur);
            }
            path_rhs.push_back(r_rhs);

            int common = 0;
            while (common < static_cast<int>(path_lhs.size()) && common < static_cast<int>(path_rhs.size()) &&
                   path_lhs[path_lhs.size() - 1 - common] == path_rhs[path_rhs.size() - 1 - common]) {
                ++common;
            }
            std::vector<int> result;
            for (std::size_t idx = 0; idx <= path_lhs.size() - common; ++idx) {
                result.push_back(path_lhs[idx]);
            }
            const int start_rhs = static_cast<int>(path_rhs.size()) - common - 1;
            for (int idx = start_rhs; idx >= 0; --idx) {
                result.push_back(path_rhs[idx]);
            }
            return result;
        }

        // Different trees on cycle
        std::vector<int> result;
        for (int cur = lhs; cur != r_lhs; cur = tree_parent_[cur]) {
            result.push_back(cur);
        }
        const std::vector<int> c_path = cycle_path(r_lhs, r_rhs);
        result.insert(result.end(), c_path.begin(), c_path.end());

        std::vector<int> path_rhs;
        for (int cur = rhs; cur != r_rhs; cur = tree_parent_[cur]) {
            path_rhs.push_back(cur);
        }
        std::reverse(path_rhs.begin(), path_rhs.end());
        result.insert(result.end(), path_rhs.begin(), path_rhs.end());
        return result;
    }

private:
    int n_;
    std::vector<int> cycle_;
    std::vector<bool> is_on_cycle_;
    std::vector<int> cycle_index_;
    std::vector<int> root_on_cycle_;
    std::vector<int> distance_to_cycle_;
    std::vector<int> tree_parent_;
};

}  // namespace cp
