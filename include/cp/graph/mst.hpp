#pragma once

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

#include "cp/ds/disjoint_set.hpp"
#include "cp/graph/graph.hpp"

namespace cp {

template <class Weight>
struct MinimumSpanningTree {
    Weight weight{};
    std::vector<typename Graph<Weight>::StoredEdge> edges;
};

template <class Weight>
std::optional<MinimumSpanningTree<Weight>> kruskal(const Graph<Weight>& graph) {
    auto edges = graph.edges();
    std::sort(edges.begin(), edges.end(), [](const auto& lhs, const auto& rhs) { return lhs.weight < rhs.weight; });
    DisjointSet components(graph.size());
    MinimumSpanningTree<Weight> result;
    for (const auto& edge : edges) {
        if (components.merge(edge.from, edge.to)) {
            result.weight += edge.weight;
            result.edges.push_back(edge);
        }
    }
    if (graph.size() > 0 && result.edges.size() + 1 != static_cast<std::size_t>(graph.size())) {
        return std::nullopt;
    }
    return result;
}

}  // namespace cp

