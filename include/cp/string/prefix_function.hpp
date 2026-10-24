#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace cp {

template <class Sequence>
std::vector<int> prefix_function(const Sequence& sequence) {
    std::vector<int> prefix(sequence.size());
    for (std::size_t index = 1; index < sequence.size(); ++index) {
        int border = prefix[index - 1];
        while (border > 0 && sequence[index] != sequence[border]) {
            border = prefix[border - 1];
        }
        if (sequence[index] == sequence[border]) {
            ++border;
        }
        prefix[index] = border;
    }
    return prefix;
}

inline std::vector<int> prefix_function(std::string_view text) {
    return prefix_function<std::string_view>(text);
}

}  // namespace cp

