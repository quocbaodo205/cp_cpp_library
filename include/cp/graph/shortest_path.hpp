#pragma once

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight>
struct ShortestPaths {
    std::vector<Weight> distance;
    std::vector<int> parent;

    std::vector<int> path_to(int target) const {
        if (target < 0 || target >= static_cast<int>(parent.size()) || parent[target] == -1) {
            return {};
        }
        std::vector<int> path;
        for (int current = target;; current = parent[current]) {
            path.push_back(current);
            if (parent[current] == current) {
                break;
            }
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

template <class Weight>
ShortestPaths<Weight> dijkstra(const Graph<Weight>& graph, int source,
                               Weight infinity = std::numeric_limits<Weight>::max() / Weight{4}) {
    assert(0 <= source && source < graph.size());
    ShortestPaths<Weight> result{std::vector<Weight>(static_cast<std::size_t>(graph.size()), infinity),
                                 std::vector<int>(static_cast<std::size_t>(graph.size()), -1)};
    using State = std::pair<Weight, int>;
    std::priority_queue<State, std::vector<State>, std::greater<>> queue;
    result.distance[source] = Weight{};
    result.parent[source] = source;
    queue.emplace(Weight{}, source);
    while (!queue.empty()) {
        const auto [distance, vertex] = queue.top();
        queue.pop();
        if (distance != result.distance[vertex]) {
            continue;
        }
        for (const Edge<Weight>& edge : graph[vertex]) {
            assert(edge.weight >= Weight{});
            if (distance <= infinity - edge.weight &&
                distance + edge.weight < result.distance[edge.to]) {
                result.distance[edge.to] = distance + edge.weight;
                result.parent[edge.to] = vertex;
                queue.emplace(distance + edge.weight, edge.to);
            }
        }
    }
    return result;
}

template <class Weight>
ShortestPaths<Weight> zero_one_bfs(const Graph<Weight>& graph, int source,
                                  Weight infinity = std::numeric_limits<Weight>::max() / Weight{4}) {
    assert(0 <= source && source < graph.size());
    ShortestPaths<Weight> result{std::vector<Weight>(static_cast<std::size_t>(graph.size()), infinity),
                                 std::vector<int>(static_cast<std::size_t>(graph.size()), -1)};
    std::deque<int> queue;
    result.distance[source] = Weight{};
    result.parent[source] = source;
    queue.push_back(source);
    while (!queue.empty()) {
        const int vertex = queue.front();
        queue.pop_front();
        for (const Edge<Weight>& edge : graph[vertex]) {
            assert(edge.weight == Weight{} || edge.weight == Weight{1});
            const Weight candidate = result.distance[vertex] + edge.weight;
            if (candidate >= result.distance[edge.to]) {
                continue;
            }
            result.distance[edge.to] = candidate;
            result.parent[edge.to] = vertex;
            if (edge.weight == Weight{}) {
                queue.push_front(edge.to);
            } else {
                queue.push_back(edge.to);
            }
        }
    }
    return result;
}

template <class Weight>
std::optional<ShortestPaths<Weight>> bellman_ford(
    const Graph<Weight>& graph, int source, Weight infinity = std::numeric_limits<Weight>::max() / Weight{4}) {
    assert(0 <= source && source < graph.size());
    ShortestPaths<Weight> result{std::vector<Weight>(static_cast<std::size_t>(graph.size()), infinity),
                                 std::vector<int>(static_cast<std::size_t>(graph.size()), -1)};
    result.distance[source] = Weight{};
    result.parent[source] = source;
    for (int iteration = 0; iteration < graph.size(); ++iteration) {
        bool changed = false;
        for (const auto& edge : graph.edges()) {
            if (result.distance[edge.from] == infinity) {
                continue;
            }
            const Weight candidate = result.distance[edge.from] + edge.weight;
            if (candidate < result.distance[edge.to]) {
                if (iteration + 1 == graph.size()) {
                    return std::nullopt;
                }
                result.distance[edge.to] = candidate;
                result.parent[edge.to] = edge.from;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }
    }
    return result;
}

template <class Weight>
std::vector<std::vector<Weight>> floyd_warshall(
    const Graph<Weight>& graph, Weight infinity = std::numeric_limits<Weight>::max() / Weight{4}) {
    const std::size_t n = static_cast<std::size_t>(graph.size());
    std::vector distance(n, std::vector<Weight>(n, infinity));
    for (std::size_t vertex = 0; vertex < n; ++vertex) {
        distance[vertex][vertex] = Weight{};
    }
    for (int from = 0; from < graph.size(); ++from) {
        for (const Edge<Weight>& edge : graph[from]) {
            distance[from][edge.to] = std::min(distance[from][edge.to], edge.weight);
        }
    }
    for (std::size_t middle = 0; middle < n; ++middle) {
        for (std::size_t from = 0; from < n; ++from) {
            if (distance[from][middle] == infinity) {
                continue;
            }
            for (std::size_t to = 0; to < n; ++to) {
                if (distance[middle][to] != infinity) {
                    distance[from][to] = std::min(distance[from][to], distance[from][middle] + distance[middle][to]);
                }
            }
        }
    }
    return distance;
}

}  // namespace cp

