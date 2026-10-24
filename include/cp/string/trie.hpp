#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <string_view>
#include <vector>

namespace cp {

template <std::size_t Alphabet = 26, char First = 'a'> class Trie {
    struct Node {
        std::array<int, Alphabet> next{};
        int prefix_count = 0;
        int terminal_count = 0;

        Node() {
            next.fill(-1);
        }
    };

  public:
    Trie() : nodes_(1) {}

    int insert(std::string_view text) {
        int node = 0;
        ++nodes_[0].prefix_count;
        for (char character : text) {
            const std::size_t edge = index(character);
            if (nodes_[node].next[edge] == -1) {
                nodes_[node].next[edge] = static_cast<int>(nodes_.size());
                nodes_.emplace_back();
            }
            node = nodes_[node].next[edge];
            ++nodes_[node].prefix_count;
        }
        ++nodes_[node].terminal_count;
        return node;
    }

    bool contains(std::string_view text) const {
        const int node = find_node(text);
        return node != -1 && nodes_[node].terminal_count > 0;
    }

    int count(std::string_view text) const {
        const int node = find_node(text);
        return node == -1 ? 0 : nodes_[node].terminal_count;
    }

    int count_prefix(std::string_view prefix) const {
        const int node = find_node(prefix);
        return node == -1 ? 0 : nodes_[node].prefix_count;
    }

    std::size_t node_count() const {
        return nodes_.size();
    }

    std::vector<Node> nodes_;

    static std::size_t index(char character) {
        const int value = static_cast<unsigned char>(character);
        const int first = static_cast<unsigned char>(First);
        assert(value >= first && value < first + static_cast<int>(Alphabet));
        return static_cast<std::size_t>(value - first);
    }

    // Standard traversal with node, copy should need outside traversal
    int find_node(std::string_view text) const {
        int node = 0;
        for (char character : text) {
            const int next = nodes_[node].next[index(character)];
            if (next == -1) {
                return -1;
            }
            node = next;
        }
        return node;
    }
};

} // namespace cp
