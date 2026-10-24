#pragma once

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace cp {

using i64 = std::int64_t;
using u64 = std::uint64_t;
using i128 = __int128_t;
using u128 = __uint128_t;

template <class T>
using Vec = std::vector<T>;

template <class T>
using Pair = std::pair<T, T>;

template <class T>
inline constexpr T inf = std::numeric_limits<T>::max() / T{4};

}  // namespace cp

