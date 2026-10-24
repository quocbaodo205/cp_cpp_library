#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <queue>
#include <vector>

namespace cp {

template <class Capacity = long long>
class Dinic {
public:
    struct Edge {
        int to;
        int reverse;
        Capacity capacity;
    };

    explicit Dinic(int vertex_count) : graph_(static_cast<std::size_t>(vertex_count)),
                                       level_(static_cast<std::size_t>(vertex_count)),
                                       next_(static_cast<std::size_t>(vertex_count)) {
        assert(vertex_count >= 0);
    }

    void add_edge(int from, int to, Capacity capacity) {
        assert(0 <= from && from < static_cast<int>(graph_.size()));
        assert(0 <= to && to < static_cast<int>(graph_.size()));
        assert(capacity >= Capacity{});
        const int from_index = static_cast<int>(graph_[from].size());
        const int to_index = static_cast<int>(graph_[to].size());
        graph_[from].push_back({to, to_index, capacity});
        graph_[to].push_back({from, from_index, Capacity{}});
    }

    Capacity max_flow(int source, int sink, Capacity limit = std::numeric_limits<Capacity>::max()) {
        Capacity flow{};
        while (flow < limit && build_levels(source, sink)) {
            std::fill(next_.begin(), next_.end(), 0);
            while (const Capacity pushed = push(source, sink, limit - flow)) {
                flow += pushed;
            }
        }
        return flow;
    }

    std::vector<bool> min_cut(int source) const {
        std::vector<bool> reachable(graph_.size());
        std::queue<int> queue;
        reachable[source] = true;
        queue.push(source);
        while (!queue.empty()) {
            const int vertex = queue.front();
            queue.pop();
            for (const Edge& edge : graph_[vertex]) {
                if (edge.capacity > Capacity{} && !reachable[edge.to]) {
                    reachable[edge.to] = true;
                    queue.push(edge.to);
                }
            }
        }
        return reachable;
    }

private:
    std::vector<std::vector<Edge>> graph_;
    std::vector<int> level_;
    std::vector<std::size_t> next_;

    bool build_levels(int source, int sink) {
        std::fill(level_.begin(), level_.end(), -1);
        std::queue<int> queue;
        level_[source] = 0;
        queue.push(source);
        while (!queue.empty()) {
            const int vertex = queue.front();
            queue.pop();
            for (const Edge& edge : graph_[vertex]) {
                if (edge.capacity > Capacity{} && level_[edge.to] == -1) {
                    level_[edge.to] = level_[vertex] + 1;
                    queue.push(edge.to);
                }
            }
        }
        return level_[sink] != -1;
    }

    Capacity push(int vertex, int sink, Capacity available) {
        if (vertex == sink) {
            return available;
        }
        for (std::size_t& index = next_[vertex]; index < graph_[vertex].size(); ++index) {
            Edge& edge = graph_[vertex][index];
            if (edge.capacity == Capacity{} ||
                level_[edge.to] != level_[vertex] + 1) {
                continue;
            }
            const Capacity pushed = push(edge.to, sink, std::min(available, edge.capacity));
            if (pushed > Capacity{}) {
                edge.capacity -= pushed;
                graph_[edge.to][edge.reverse].capacity += pushed;
                return pushed;
            }
        }
        return Capacity{};
    }
};

}  // namespace cp

