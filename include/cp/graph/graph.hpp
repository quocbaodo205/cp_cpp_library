#pragma once

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cp {

template <class Weight = long long>
struct Edge {
    int to;
    Weight weight;
    int id;
};

template <class Weight = long long>
class Graph {
public:
    explicit Graph(int vertex_count = 0) : adjacency_(static_cast<std::size_t>(vertex_count)) {
        assert(vertex_count >= 0);
    }

    int size() const { return static_cast<int>(adjacency_.size()); }
    int edge_count() const { return edge_count_; }

    int add_directed_edge(int from, int to, Weight weight = Weight{1}) {
        check_vertex(from);
        check_vertex(to);
        const int id = edge_count_++;
        adjacency_[from].push_back(Edge<Weight>{to, weight, id});
        edges_.emplace_back(from, to, weight);
        return id;
    }

    int add_edge(int lhs, int rhs, Weight weight = Weight{1}) {
        check_vertex(lhs);
        check_vertex(rhs);
        const int id = edge_count_++;
        adjacency_[lhs].push_back(Edge<Weight>{rhs, weight, id});
        adjacency_[rhs].push_back(Edge<Weight>{lhs, weight, id});
        edges_.emplace_back(lhs, rhs, weight);
        edges_.emplace_back(rhs, lhs, weight);
        return id;
    }

    const std::vector<Edge<Weight>>& operator[](int vertex) const {
        check_vertex(vertex);
        return adjacency_[vertex];
    }

    std::vector<Edge<Weight>>& operator[](int vertex) {
        check_vertex(vertex);
        return adjacency_[vertex];
    }

    struct StoredEdge {
        int from;
        int to;
        Weight weight;
    };

    const std::vector<StoredEdge>& edges() const { return edges_; }

    Graph reversed() const {
        Graph result(size());
        for (const StoredEdge& edge : edges_) {
            result.add_directed_edge(edge.to, edge.from, edge.weight);
        }
        return result;
    }

private:
    std::vector<std::vector<Edge<Weight>>> adjacency_;
    std::vector<StoredEdge> edges_;
    int edge_count_ = 0;

    void check_vertex([[maybe_unused]] int vertex) const { assert(0 <= vertex && vertex < size()); }
};

class StringVertices {
public:
    int get_or_add(std::string_view name) {
        const auto [iterator, inserted] = ids_.try_emplace(std::string(name), static_cast<int>(names_.size()));
        if (inserted) {
            names_.push_back(iterator->first);
        }
        return iterator->second;
    }

    int find(std::string_view name) const {
        const auto iterator = ids_.find(std::string(name));
        return iterator == ids_.end() ? -1 : iterator->second;
    }

    const std::string& name(int vertex) const {
        assert(0 <= vertex && vertex < static_cast<int>(names_.size()));
        return names_[vertex];
    }

    int size() const { return static_cast<int>(names_.size()); }

private:
    std::unordered_map<std::string, int> ids_;
    std::vector<std::string> names_;
};

struct GridPoint {
    int row = 0;
    int column = 0;

    auto operator<=>(const GridPoint&) const = default;

    int id(int columns) const { return row * columns + column; }
    static GridPoint from_id(int id, int columns) { return {id / columns, id % columns}; }
};

inline constexpr int grid_dr[4] = {-1, 0, 1, 0};
inline constexpr int grid_dc[4] = {0, 1, 0, -1};

template <class Function>
void for_each_grid_neighbor(GridPoint point, int rows, int columns, Function function) {
    for (int direction = 0; direction < 4; ++direction) {
        const GridPoint next{point.row + grid_dr[direction], point.column + grid_dc[direction]};
        if (0 <= next.row && next.row < rows && 0 <= next.column && next.column < columns) {
            function(next);
        }
    }
}

}  // namespace cp

