#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight>
std::vector<int> dfs_preorder(const Graph<Weight>& graph, int source) {
    assert(0 <= source && source < graph.size());
    std::vector<int> order;
    std::vector<bool> seen(static_cast<std::size_t>(graph.size()));
    std::vector<int> stack{source};
    while (!stack.empty()) {
        const int vertex = stack.back();
        stack.pop_back();
        if (seen[vertex]) {
            continue;
        }
        seen[vertex] = true;
        order.push_back(vertex);
        for (auto iterator = graph[vertex].rbegin(); iterator != graph[vertex].rend(); ++iterator) {
            if (!seen[iterator->to]) {
                stack.push_back(iterator->to);
            }
        }
    }
    return order;
}

template <class Weight>
std::vector<int> bfs_distance(const Graph<Weight>& graph, const std::vector<int>& sources) {
    std::vector<int> distance(static_cast<std::size_t>(graph.size()), -1);
    std::queue<int> queue;
    for (int source : sources) {
        assert(0 <= source && source < graph.size());
        if (distance[source] == -1) {
            distance[source] = 0;
            queue.push(source);
        }
    }
    while (!queue.empty()) {
        const int vertex = queue.front();
        queue.pop();
        for (const Edge<Weight>& edge : graph[vertex]) {
            if (distance[edge.to] == -1) {
                distance[edge.to] = distance[vertex] + 1;
                queue.push(edge.to);
            }
        }
    }
    return distance;
}

template <class Weight>
std::vector<int> bfs_distance(const Graph<Weight>& graph, int source) {
    return bfs_distance(graph, std::vector<int>{source});
}

template <class Weight>
std::optional<std::vector<int>> topological_sort(const Graph<Weight>& graph) {
    std::vector<int> indegree(static_cast<std::size_t>(graph.size()));
    for (int vertex = 0; vertex < graph.size(); ++vertex) {
        for (const Edge<Weight>& edge : graph[vertex]) {
            ++indegree[edge.to];
        }
    }
    std::priority_queue<int, std::vector<int>, std::greater<>> ready;
    for (int vertex = 0; vertex < graph.size(); ++vertex) {
        if (indegree[vertex] == 0) {
            ready.push(vertex);
        }
    }
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(graph.size()));
    while (!ready.empty()) {
        const int vertex = ready.top();
        ready.pop();
        order.push_back(vertex);
        for (const Edge<Weight>& edge : graph[vertex]) {
            if (--indegree[edge.to] == 0) {
                ready.push(edge.to);
            }
        }
    }
    if (static_cast<int>(order.size()) != graph.size()) {
        return std::nullopt;
    }
    return order;
}

struct StronglyConnectedComponents {
    int count = 0;
    std::vector<int> component;
    std::vector<std::vector<int>> groups;
};

template <class Weight>
StronglyConnectedComponents strongly_connected_components(const Graph<Weight>& graph) {
    const int n = graph.size();
    std::vector<bool> seen(static_cast<std::size_t>(n));
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(n));
    const auto dfs = [&](auto&& self, int vertex) -> void {
        seen[vertex] = true;
        for (const Edge<Weight>& edge : graph[vertex]) {
            if (!seen[edge.to]) {
                self(self, edge.to);
            }
        }
        order.push_back(vertex);
    };
    for (int vertex = 0; vertex < n; ++vertex) {
        if (!seen[vertex]) {
            dfs(dfs, vertex);
        }
    }

    const Graph<Weight> reversed = graph.reversed();
    StronglyConnectedComponents result{0, std::vector<int>(static_cast<std::size_t>(n), -1), {}};
    const auto assign = [&](auto&& self, int vertex, int component) -> void {
        result.component[vertex] = component;
        result.groups[component].push_back(vertex);
        for (const Edge<Weight>& edge : reversed[vertex]) {
            if (result.component[edge.to] == -1) {
                self(self, edge.to, component);
            }
        }
    };
    std::reverse(order.begin(), order.end());
    for (int vertex : order) {
        if (result.component[vertex] == -1) {
            result.groups.emplace_back();
            assign(assign, vertex, result.count++);
        }
    }
    return result;
}

template <class Weight>
std::vector<int> connected_components(const Graph<Weight>& graph) {
    std::vector<int> component(static_cast<std::size_t>(graph.size()), -1);
    int component_id = 0;
    for (int start = 0; start < graph.size(); ++start) {
        if (component[start] != -1) {
            continue;
        }
        std::queue<int> queue;
        queue.push(start);
        component[start] = component_id;
        while (!queue.empty()) {
            const int vertex = queue.front();
            queue.pop();
            for (const Edge<Weight>& edge : graph[vertex]) {
                if (component[edge.to] == -1) {
                    component[edge.to] = component_id;
                    queue.push(edge.to);
                }
            }
        }
        ++component_id;
    }
    return component;
}

}  // namespace cp

