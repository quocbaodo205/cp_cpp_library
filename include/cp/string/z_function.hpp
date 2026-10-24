#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <vector>

namespace cp {

template <class Sequence>
std::vector<int> z_function(const Sequence& sequence) {
    const int n = static_cast<int>(sequence.size());
    std::vector<int> z(sequence.size());
    for (int i = 1, left = 0, right = 0; i < n; ++i) {
        if (i < right) {
            z[i] = std::min(right - i, z[i - left]);
        }
        while (i + z[i] < n && sequence[z[i]] == sequence[i + z[i]]) {
            ++z[i];
        }
        if (i + z[i] > right) {
            left = i;
            right = i + z[i];
        }
    }
    if (n > 0) {
        z[0] = n;
    }
    return z;
}

inline std::vector<int> z_function(std::string_view text) { return z_function<std::string_view>(text); }

}  // namespace cp

