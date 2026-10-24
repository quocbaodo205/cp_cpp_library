# C++ Competitive Programming Workspace

This workspace uses LLVM Clang 22.1.6 with C++23, clangd for editor intelligence, and a header-only competitive-programming library that can be flattened into one submission file.

## First build

```sh
cmake --preset clang-debug
cmake --build --preset clang-debug
printf '5\n1 2 3 4 5\n' | ./build/clang-debug/cp_main
```

The expected output from the starter solution is `15`. For an optimized local build, use the `clang-release` preset instead.

## clangd

Run `cmake --preset clang-debug` once to generate `build/clang-debug/compile_commands.json`. The checked-in `.clangd` points clangd at that database and tells it to parse the code as C++23 using Homebrew LLVM Clang.

The checked-in `.vscode/settings.json` uses Homebrew clangd and lets it query the matching compiler. In another editor, launch clangd with:

```text
--query-driver=/opt/homebrew/opt/llvm/bin/clang++
```

## Adding library modules

Keep reusable code header-only under `include/cp/`, grouped by subject:

```text
include/cp/
├── ds/
│   └── disjoint_set.hpp
├── graph/
│   └── shortest_path.hpp
├── io.hpp
└── types.hpp
```

A module should use `#pragma once`, include its own dependencies, and put public names in `namespace cp`. Then include only what the solution needs:

```cpp
#include "cp/ds/disjoint_set.hpp"

cp::DisjointSet components(n);
```

This plays the same role as the independent `src/*.zig` modules in `zig_algo`. `include/cp/all.hpp` is available for experiments, but focused includes produce smaller submissions. Header-only code is intentional: templates work naturally, CMake exposes one include root, and bundling does not need a linker or special handling for implementation files.

## Library catalog

Include the narrow header listed below, a family header such as `cp/graph.hpp`, or `cp/all.hpp`.

| Header | Main APIs |
| --- | --- |
| `cp/types.hpp` | `i64`, `u64`, `i128`, `u128`, `Vec`, `Pair`, `inf` |
| `cp/io.hpp` | `init_io` for fast `std::cin` and `std::cout` |
| `cp/memo.hpp` | recursive memoization with tuple keys |
| `cp/utils.hpp` | `chmin`, `chmax`, signed floor/ceil division, binary power, bound indices, coordinate compression, recursive lambda, SplitMix64 and Zobrist hashes |
| `cp/ds/disjoint_set.hpp` | union-find with component sizes and group extraction |
| `cp/ds/persistent_disjoint_set.hpp` | immutable union-find versions |
| `cp/ds/persistent_array.hpp` | immutable point updates |
| `cp/ds/fenwick.hpp` | additive Fenwick tree, monoid Fenwick tree, ordered multiset |
| `cp/ds/segment_tree.hpp` | generic monoid segment tree, `max_right`, `min_left` |
| `cp/ds/lazy_segment_tree.hpp` | generic lazy segment tree and range-add/range-sum action |
| `cp/ds/sparse_table.hpp` | static idempotent range queries |
| `cp/ds/cartesian_tree.hpp` | min/max Cartesian trees through a comparator |
| `cp/ds/fixed_deque.hpp` | allocation-free bounded deque |
| `cp/string/trie.hpp` | counted alphabet trie |
| `cp/string/z_function.hpp` | Z-function |
| `cp/string/prefix_function.hpp` | KMP prefix function |
| `cp/string/rolling_hash.hpp` | 61-bit substring hash |
| `cp/graph/graph.hpp` | weighted directed/undirected adjacency lists, grid points, string vertex IDs |
| `cp/graph/traversal.hpp` | DFS, BFS, topological sort, connected components, SCC |
| `cp/graph/shortest_path.hpp` | Dijkstra, 0-1 BFS, Bellman-Ford, Floyd-Warshall |
| `cp/graph/tree.hpp` | binary-lifting LCA, tree distances, diameter path, tree centers |
| `cp/graph/rooted_tree.hpp` | tree parent/depth/size/order, in/out timestamps, LCA, path extraction, DSU on tree (Sack) |
| `cp/graph/dfs_tree.hpp` | fast `SimpleDfsTree`, full undirected & directed DFS trees, bridges, 2-edge-connected components, back/cross edges |
| `cp/graph/hld.hpp` | Heavy-Light Decomposition, path & subtree range intervals, LCA, kth ancestor |
| `cp/graph/centroid_decomposition.hpp` | centroid tree, logarithmic height decomposition, centroid ancestor traversal |
| `cp/graph/ring_graph.hpp` | pseudotree / ring graph cycle peeling, cycle distance & path, attached tree queries |
| `cp/graph/mst.hpp` | Kruskal minimum spanning tree |
| `cp/graph/bipartite_matching.hpp` | maximum bipartite matching |
| `cp/graph/flow.hpp` | Dinic maximum flow and minimum-cut side |
| `cp/math/modint.hpp` | compile-time modular integer types for `998244353` and `1e9+7` |
| `cp/math/combinatorics.hpp` | exact and modular combinations, Lucas theorem, compositions, permutation cycles |
| `cp/math/prime.hpp` | linear sieve, factorization, deterministic 64-bit Miller-Rabin, primitive roots |
| `cp/math/number_theory.hpp` | extended GCD, modular inverse/power, floor sum, quotient grouping, Möbius/totient sieves, divisors, CRT, discrete log, multiplicative order, radix blocks |
| `cp/math/fft.hpp` | floating-point and rounded integer convolution |
| `cp/math/ntt.hpp` | generic NTT and `998244353` convolution |
| `cp/math/formal_power_series.hpp` | FPS arithmetic, derivative/integral, inverse, logarithm, exponential, power, square root, pairwise convolution sum |
| `cp/math/matrix.hpp` | fixed-size matrix multiplication, vector application, exponentiation |

Most range APIs use 0-based half-open intervals `[left, right)`. Graph vertices are 0-based. Use `std::optional` results to detect missing inverses, incompatible CRT systems, negative cycles, and absent discrete logarithms.

```cpp
#include <iostream>
#include <vector>

#include "cp/ds/monoids.hpp"
#include "cp/ds/segment_tree.hpp"
#include "cp/io.hpp"

int main() {
    cp::init_io();
    cp::SegmentTree<cp::SumMonoid<long long>> tree(
        std::vector<long long>{1, 2, 3, 4});
    std::cout << tree.fold(1, 4) << '\n';
}
```

## Library checks

```sh
cmake --build --preset clang-debug --target cp_library_test
ctest --test-dir build/clang-debug --output-on-failure
```

The test target compiles with warnings promoted to errors and instantiates the umbrella header.

## Abseil and Boost

The submission environment permits Abseil and Boost. `solutions/main.cpp` contains commented include reminders.

Use Abseil containers for these cases:

```cpp
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/btree_map.h>
#include <absl/container/btree_set.h>

absl::flat_hash_map<int, int> frequency; // Hash table.
absl::btree_map<int, int> ordered;       // Sorted keys and range queries.
```

Useful Boost components for CP include:

```cpp
#include <boost/multiprecision/cpp_int.hpp> // Integers larger than 128 bits.
#include <boost/dynamic_bitset.hpp>         // Runtime-sized bitsets.
#include <boost/geometry.hpp>               // Specialized geometry operations.
```

Prefer the standard library or `include/cp/` for routine problems. Pull in Boost by component because umbrella headers such as `boost/geometry.hpp` can increase compile time. The bundler leaves angle-bracket includes unchanged, so Abseil and Boost remain external dependencies supplied by the judge.

## Producing one submission file

Generate the flattened source without compiling it:

```sh
cmake --build --preset clang-debug --target bundle
```

The result is `build/clang-debug/submission.cpp`. The bundler recursively replaces quoted local includes such as `#include "cp/graph/shortest_path.hpp"`, removes `#pragma once`, emits each local header once, and strips standalone `#ifdef CP_VIZ` blocks. Standard-library includes such as `#include <vector>` remain intact for the judge compiler.

Compile the flattened file as an extra safety check:

```sh
cmake --preset clang-release
cmake --build --preset clang-release --target submission
```

That command first creates `build/clang-release/submission.cpp`, then compiles the self-contained source into `build/clang-release/submission`. Submit the generated `.cpp`, not the executable.

Use quoted includes for project headers and angle brackets for system headers. The bundler deliberately fails on an unresolved quoted include so it cannot silently produce a broken submission. It is a textual include flattener, so keep reusable CP modules in headers rather than C++20 named modules or separately compiled `.cpp` files.
