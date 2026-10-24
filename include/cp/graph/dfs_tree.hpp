#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

/**
 * Traversal & Reference Notes (from rust_algo):
 *
 * 1. Bottom-to-Top Traversal (Leaves to Root):
 *    Process vertices in descending order of depth (via order_by_depth_desc()).
 *    Useful for tree DP, aggregating subtree properties, and eliminating edges.
 *    ```cpp
 *    for (int u : tree.order_by_depth_desc()) {
 *        // Process u bottom-up
 *    }
 *    ```
 *
 * 2. Back Edge Level Ordering:
 *    back_edges are oriented (ancestor, descendant) with depth[u] < depth[v]
 *    and sorted by depth[ancestor] ascending.
 *    When ascending tree chains from descendant v up to ancestor u, sorting
 *    ensures higher-level chains are considered consistently.
 *
 * 3. Grouping Back Edges:
 *    ```cpp
 *    std::vector<std::vector<int>> be_ancestor(n);   // ancestor u -> list of descendants v
 *    std::vector<std::vector<int>> be_descendant(n); // descendant v -> list of ancestors u
 *    for (const auto& [u, v] : tree.back_edges) {
 *        be_ancestor[u].push_back(v);
 *        be_descendant[v].push_back(u);
 *    }
 *    ```
 *
 * 4. Climbing Tree Chains:
 *    For each back-edge (u, v), the cycle consists of tree path v -> u plus edge (u, v):
 *    ```cpp
 *    for (const auto& [u, v] : tree.back_edges) {
 *        int cur = v;
 *        while (cur != u && cur != -1) {
 *            int p = tree.parent[cur];
 *            // Tree edge (p, cur) is covered by back-edge cycle (u, v)
 *            cur = p;
 *        }
 *    }
 *    ```
 */

struct SimpleDfsTree {
    std::vector<int> depth;
    std::vector<int> parent;
    std::vector<std::pair<int, int>> span_edges;
    std::vector<std::pair<int, int>> back_edges; // (ancestor, descendant) with depth[u] < depth[v]

    int size() const { return static_cast<int>(depth.size()); }

    std::vector<int> order_by_depth_desc() const {
        std::vector<int> order(depth.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int lhs, int rhs) { return depth[lhs] > depth[rhs]; });
        return order;
    }

    std::vector<int> order_by_depth_asc() const {
        std::vector<int> order(depth.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int lhs, int rhs) { return depth[lhs] < depth[rhs]; });
        return order;
    }
};

template <class Weight = long long>
SimpleDfsTree build_simple_dfs_tree(const Graph<Weight>& graph, int root = 0) {
    const int n = graph.size();
    SimpleDfsTree result;
    result.depth.assign(static_cast<std::size_t>(n), -1);
    result.parent.assign(static_cast<std::size_t>(n), -1);

    struct Frame {
        int vertex;
        int parent_edge;
        int next_edge_idx;
    };
    std::vector<Frame> stack;
    stack.reserve(static_cast<std::size_t>(n));

    const auto run_dfs = [&](int start) {
        result.depth[start] = 0;
        result.parent[start] = -1;
        stack.push_back({start, -1, 0});

        while (!stack.empty()) {
            const int u = stack.back().vertex;
            const int p_edge = stack.back().parent_edge;
            bool pushed = false;

            while (stack.back().next_edge_idx < static_cast<int>(graph[u].size())) {
                const int edge_idx = stack.back().next_edge_idx++;
                const Edge<Weight>& edge = graph[u][edge_idx];
                if (edge.id == p_edge) {
                    continue;
                }
                const int to = edge.to;
                if (result.depth[to] == -1) {
                    result.span_edges.emplace_back(u, to);
                    result.parent[to] = u;
                    result.depth[to] = result.depth[u] + 1;
                    stack.push_back({to, edge.id, 0});
                    pushed = true;
                    break;
                } else if (result.depth[to] < result.depth[u]) {
                    result.back_edges.emplace_back(to, u);
                }
            }

            if (!pushed) {
                stack.pop_back();
            }
        }
    };

    if (n > 0 && 0 <= root && root < n) {
        run_dfs(root);
    }
    for (int vertex = 0; vertex < n; ++vertex) {
        if (result.depth[vertex] == -1) {
            run_dfs(vertex);
        }
    }
    return result;
}

inline SimpleDfsTree build_simple_dfs_tree(const std::vector<std::vector<int>>& adj, int root = 0) {
    const int n = static_cast<int>(adj.size());
    SimpleDfsTree result;
    result.depth.assign(static_cast<std::size_t>(n), -1);
    result.parent.assign(static_cast<std::size_t>(n), -1);

    struct Frame {
        int vertex;
        int parent;
        int next_edge_idx;
    };
    std::vector<Frame> stack;
    stack.reserve(static_cast<std::size_t>(n));

    const auto run_dfs = [&](int start) {
        result.depth[start] = 0;
        result.parent[start] = -1;
        stack.push_back({start, -1, 0});

        while (!stack.empty()) {
            const int u = stack.back().vertex;
            const int p = stack.back().parent;
            bool pushed = false;

            while (stack.back().next_edge_idx < static_cast<int>(adj[u].size())) {
                const int to = adj[u][stack.back().next_edge_idx++];
                if (to == p) {
                    continue;
                }
                if (result.depth[to] == -1) {
                    result.span_edges.emplace_back(u, to);
                    result.parent[to] = u;
                    result.depth[to] = result.depth[u] + 1;
                    stack.push_back({to, u, 0});
                    pushed = true;
                    break;
                } else if (result.depth[to] < result.depth[u]) {
                    result.back_edges.emplace_back(to, u);
                }
            }

            if (!pushed) {
                stack.pop_back();
            }
        }
    };

    if (n > 0 && 0 <= root && root < n) {
        run_dfs(root);
    }
    for (int vertex = 0; vertex < n; ++vertex) {
        if (result.depth[vertex] == -1) {
            run_dfs(vertex);
        }
    }
    return result;
}

template <class Weight = long long> struct DfsTreeUndirected {
    std::vector<std::pair<int, int>> span_edges;
    std::vector<std::pair<int, int>> back_edges; // (ancestor, descendant) with depth[u] < depth[v]
    std::vector<std::pair<int, int>> bridges;
    std::vector<int> depth;
    std::vector<int> parent;
    std::vector<int> time_in;
    std::vector<int> time_out;
    std::vector<int> low;
    std::vector<int> two_edge_connected_components;
    int two_edge_connected_count = 0;

    int size() const {
        return static_cast<int>(depth.size());
    }

    bool is_bridge(int u, int v) const {
        if (depth[u] > depth[v]) {
            std::swap(u, v);
        }
        return parent[v] == u && low[v] > time_in[u];
    }

    /**
     * Returns vertices ordered by depth descending (bottom to top, leaves to root).
     */
    std::vector<int> order_by_depth_desc() const {
        std::vector<int> order(depth.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int lhs, int rhs) { return depth[lhs] > depth[rhs]; });
        return order;
    }

    /**
     * Returns vertices ordered by depth ascending (top to bottom, root to leaves).
     */
    std::vector<int> order_by_depth_asc() const {
        std::vector<int> order(depth.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int lhs, int rhs) { return depth[lhs] < depth[rhs]; });
        return order;
    }
};

template <class Weight = long long>
DfsTreeUndirected<Weight> build_dfs_tree_undirected(const Graph<Weight> &graph, int root = 0) {
    const int n = graph.size();
    DfsTreeUndirected<Weight> result;
    result.depth.assign(static_cast<std::size_t>(n), 0);
    result.parent.assign(static_cast<std::size_t>(n), -1);
    result.time_in.assign(static_cast<std::size_t>(n), -1);
    result.time_out.assign(static_cast<std::size_t>(n), -1);
    result.low.assign(static_cast<std::size_t>(n), -1);
    result.two_edge_connected_components.assign(static_cast<std::size_t>(n), -1);

    int timer = 0;
    std::vector<bool> visited_edge(static_cast<std::size_t>(graph.edge_count()), false);
    std::vector<bool> bridge_edge(static_cast<std::size_t>(graph.edge_count()), false);

    struct Frame {
        int vertex;
        int parent_edge;
        int next_edge_idx;
    };
    std::vector<Frame> stack;
    stack.reserve(static_cast<std::size_t>(n));

    const auto run_dfs = [&](int start) {
        result.time_in[start] = timer++;
        result.low[start] = result.time_in[start];
        result.depth[start] = 0;
        result.parent[start] = -1;
        stack.push_back({start, -1, 0});

        while (!stack.empty()) {
            const int u = stack.back().vertex;
            const int p_edge = stack.back().parent_edge;
            bool pushed = false;

            while (stack.back().next_edge_idx < static_cast<int>(graph[u].size())) {
                const int edge_idx = stack.back().next_edge_idx++;
                const Edge<Weight> &edge = graph[u][edge_idx];
                if (edge.id == p_edge || visited_edge[edge.id]) {
                    continue;
                }
                visited_edge[edge.id] = true;

                const int to = edge.to;
                if (result.time_in[to] != -1) {
                    // Back edge between vertex u and ancestor to
                    // Store normalized as (ancestor, descendant) where depth[to] < depth[u]
                    result.back_edges.emplace_back(to, u);
                    result.low[u] = std::min(result.low[u], result.time_in[to]);
                } else {
                    result.span_edges.emplace_back(u, to);
                    result.parent[to] = u;
                    result.depth[to] = result.depth[u] + 1;
                    result.time_in[to] = timer++;
                    result.low[to] = result.time_in[to];
                    stack.push_back({to, edge.id, 0});
                    pushed = true;
                    break;
                }
            }

            if (pushed) {
                continue;
            }

            const int cur = stack.back().vertex;
            const int cur_parent_edge = stack.back().parent_edge;
            result.time_out[cur] = timer;
            stack.pop_back();

            if (!stack.empty()) {
                const int p = stack.back().vertex;
                result.low[p] = std::min(result.low[p], result.low[cur]);
                if (result.low[cur] > result.time_in[p]) {
                    result.bridges.emplace_back(p, cur);
                    if (cur_parent_edge != -1) {
                        bridge_edge[cur_parent_edge] = true;
                    }
                }
            }
        }
    };

    if (n > 0 && 0 <= root && root < n) {
        run_dfs(root);
    }
    for (int vertex = 0; vertex < n; ++vertex) {
        if (result.time_in[vertex] == -1) {
            run_dfs(vertex);
        }
    }

    // Sort back edges by depth of ancestor ascending
    std::sort(result.back_edges.begin(), result.back_edges.end(),
              [&](const std::pair<int, int> &lhs, const std::pair<int, int> &rhs) {
                  if (result.depth[lhs.first] != result.depth[rhs.first]) {
                      return result.depth[lhs.first] < result.depth[rhs.first];
                  }
                  return result.depth[lhs.second] < result.depth[rhs.second];
              });

    // Compute 2-edge-connected components (condensing bridges)
    int comp_id = 0;
    for (int start = 0; start < n; ++start) {
        if (result.two_edge_connected_components[start] != -1) {
            continue;
        }
        std::queue<int> queue;
        queue.push(start);
        result.two_edge_connected_components[start] = comp_id;
        while (!queue.empty()) {
            const int cur = queue.front();
            queue.pop();
            for (const Edge<Weight> &edge : graph[cur]) {
                if (bridge_edge[edge.id]) {
                    continue;
                }
                if (result.two_edge_connected_components[edge.to] == -1) {
                    result.two_edge_connected_components[edge.to] = comp_id;
                    queue.push(edge.to);
                }
            }
        }
        ++comp_id;
    }
    result.two_edge_connected_count = comp_id;

    return result;
}

template <class Weight = long long> struct DfsTreeDirected {
    std::vector<std::pair<int, int>> tree_edges;
    std::vector<std::pair<int, int>> back_edges;
    std::vector<std::pair<int, int>> forward_edges;
    std::vector<std::pair<int, int>> cross_edges;
    std::vector<int> depth;
    std::vector<int> parent;
    std::vector<int> time_in;
    std::vector<int> time_out;

    int size() const {
        return static_cast<int>(depth.size());
    }

    std::vector<int> order_by_depth_desc() const {
        std::vector<int> order(depth.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int lhs, int rhs) { return depth[lhs] > depth[rhs]; });
        return order;
    }

    std::vector<int> order_by_depth_asc() const {
        std::vector<int> order(depth.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int lhs, int rhs) { return depth[lhs] < depth[rhs]; });
        return order;
    }
};

template <class Weight = long long>
DfsTreeDirected<Weight> build_dfs_tree_directed(const Graph<Weight> &graph, int root = 0) {
    const int n = graph.size();
    DfsTreeDirected<Weight> result;
    result.depth.assign(static_cast<std::size_t>(n), 0);
    result.parent.assign(static_cast<std::size_t>(n), -1);
    result.time_in.assign(static_cast<std::size_t>(n), -1);
    result.time_out.assign(static_cast<std::size_t>(n), -1);

    enum class State : uint8_t { Unvisited, Visiting, Visited };
    std::vector<State> state(static_cast<std::size_t>(n), State::Unvisited);
    int timer = 0;

    struct Frame {
        int vertex;
        int next_edge_idx;
    };
    std::vector<Frame> stack;
    stack.reserve(static_cast<std::size_t>(n));

    const auto run_dfs = [&](int start) {
        state[start] = State::Visiting;
        result.time_in[start] = timer++;
        result.depth[start] = 0;
        result.parent[start] = -1;
        stack.push_back({start, 0});

        while (!stack.empty()) {
            const int u = stack.back().vertex;
            bool pushed = false;

            while (stack.back().next_edge_idx < static_cast<int>(graph[u].size())) {
                const int edge_idx = stack.back().next_edge_idx++;
                const Edge<Weight> &edge = graph[u][edge_idx];
                const int to = edge.to;

                if (state[to] == State::Unvisited) {
                    result.tree_edges.emplace_back(u, to);
                    result.parent[to] = u;
                    result.depth[to] = result.depth[u] + 1;
                    state[to] = State::Visiting;
                    result.time_in[to] = timer++;
                    stack.push_back({to, 0});
                    pushed = true;
                    break;
                } else if (state[to] == State::Visiting) {
                    // Edge to an ancestor currently on the recursion stack
                    result.back_edges.emplace_back(u, to);
                } else {
                    // Already visited
                    if (result.time_in[u] < result.time_in[to]) {
                        // Descendant visited earlier (e.g. via another branch)
                        result.forward_edges.emplace_back(u, to);
                    } else {
                        // Cross edge to a different subtree
                        result.cross_edges.emplace_back(u, to);
                    }
                }
            }

            if (pushed) {
                continue;
            }

            const int cur = stack.back().vertex;
            result.time_out[cur] = timer++;
            state[cur] = State::Visited;
            stack.pop_back();
        }
    };

    if (n > 0 && 0 <= root && root < n) {
        run_dfs(root);
    }
    for (int vertex = 0; vertex < n; ++vertex) {
        if (state[vertex] == State::Unvisited) {
            run_dfs(vertex);
        }
    }

    return result;
}

} // namespace cp
