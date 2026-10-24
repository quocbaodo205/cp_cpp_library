#pragma once

#include <cassert>
#include <vector>

namespace cp {

class BipartiteMatching {
public:
    BipartiteMatching(int left_size, int right_size)
        : adjacency_(static_cast<std::size_t>(left_size)), match_left_(static_cast<std::size_t>(left_size), -1),
          match_right_(static_cast<std::size_t>(right_size), -1) {
        assert(left_size >= 0 && right_size >= 0);
    }

    void add_edge(int left, int right) {
        assert(0 <= left && left < static_cast<int>(adjacency_.size()));
        assert(0 <= right && right < static_cast<int>(match_right_.size()));
        adjacency_[left].push_back(right);
    }

    int maximum_matching() {
        std::fill(match_left_.begin(), match_left_.end(), -1);
        std::fill(match_right_.begin(), match_right_.end(), -1);
        int result = 0;
        for (int left = 0; left < static_cast<int>(adjacency_.size()); ++left) {
            std::vector<bool> seen(match_right_.size());
            if (augment(left, seen)) {
                ++result;
            }
        }
        return result;
    }

    const std::vector<int>& left_matches() const { return match_left_; }
    const std::vector<int>& right_matches() const { return match_right_; }

private:
    std::vector<std::vector<int>> adjacency_;
    std::vector<int> match_left_;
    std::vector<int> match_right_;

    bool augment(int left, std::vector<bool>& seen) {
        for (int right : adjacency_[left]) {
            if (seen[right]) {
                continue;
            }
            seen[right] = true;
            if (match_right_[right] == -1 || augment(match_right_[right], seen)) {
                match_left_[left] = right;
                match_right_[right] = left;
                return true;
            }
        }
        return false;
    }
};

}  // namespace cp

