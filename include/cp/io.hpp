#pragma once

#include <iostream>

namespace cp {

inline void init_io() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
}

}  // namespace cp


