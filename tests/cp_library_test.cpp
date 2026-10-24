#include "cp/all.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* expression, const char* test, int line) {
    if (!condition) {
        std::cerr << test << ':' << line << ": check failed: " << expression << '\n';
        ++failures;
    }
}

#define CHECK(expression) check(static_cast<bool>(expression), #expression, __func__, __LINE__)

void test_utils() {
    int value = 10;
    CHECK(cp::chmin(value, 4));
    CHECK(value == 4);
    CHECK(!cp::chmax(value, 3));
    CHECK(cp::floor_div(-7, 3) == -3);
    CHECK(cp::ceil_div(-7, 3) == -2);
    CHECK(cp::binary_power(3LL, 4) == 81);
    CHECK(cp::lower_bound_index(std::vector{1, 2, 2, 5}, 2) == 1);
    CHECK(cp::upper_bound_index(std::vector{1, 2, 2, 5}, 2) == 3);
    CHECK((cp::coordinate_compress(std::vector{40, 10, 40, 20}) == std::vector<std::size_t>{2, 0, 2, 1}));
    CHECK((cp::transpose(std::vector<std::string>{"abc", "def"}) == std::vector<std::string>{"ad", "be", "cf"}));
    CHECK((cp::transpose(std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5, 6}}) ==
           std::vector<std::vector<int>>{{1, 3, 5}, {2, 4, 6}}));
    cp::YCombinator factorial([](auto self, int n) -> int { return n <= 1 ? 1 : n * self(n - 1); });
    CHECK(factorial(6) == 720);
    const cp::Zobrist128 zobrist(42);
    cp::ZobristHash128 state;
    zobrist.replace(state, 7, 0, 2);
    CHECK(state == zobrist.token(7, 2));
    zobrist.replace(state, 7, 2, 0);
    CHECK(state == cp::ZobristHash128{});
}

void test_recursive_memoization() {
    int calls = 0;
    auto fibonacci = cp::memoize_recursive<long long(int)>(
        [&](auto&& self, int n) -> long long {
            ++calls;
            return n <= 1 ? n : self(n - 1) + self(n - 2);
        });
    fibonacci.reserve(50);
    CHECK(fibonacci(40) == 102334155);
    CHECK(calls == 41);
    CHECK(fibonacci.size() == 41);
    CHECK(fibonacci.contains(40));
    CHECK(fibonacci(40) == 102334155 && calls == 41);
    fibonacci.clear();
    CHECK(fibonacci.size() == 0);

    auto grid_paths = cp::memoize_recursive<long long(int, int)>(
        [&](auto&& self, int row, int column) -> long long {
            if (row < 0 || column < 0) {
                return 0;
            }
            if (row == 0 && column == 0) {
                return 1;
            }
            return self(row - 1, column) + self(row, column - 1);
        });
    CHECK(grid_paths(3, 4) == 35);
    CHECK(grid_paths.contains(3, 4));
}

void test_basic_data_structures() {
    cp::FixedDeque<int, 4> deque;
    deque.push_back(2);
    deque.push_front(1);
    deque.push_back(3);
    CHECK(deque.front() == 1 && deque.back() == 3);
    CHECK(deque.pop_front() == 1);
    CHECK(deque.pop_back() == 3);
    CHECK(deque.pop_back() == 2 && deque.empty());

    const std::vector values{3, 1, 4, 1, 5};
    const auto tree = cp::make_cartesian_tree<int>(values);
    CHECK(tree.root == 1);
    CHECK(tree.parent[3] == 1);

    cp::DisjointSet dsu(5);
    CHECK(dsu.components() == 5);
    CHECK(dsu.merge(0, 1));
    CHECK(dsu.merge(1, 2));
    CHECK(!dsu.merge(0, 2));
    CHECK(dsu.same(0, 2));
    CHECK(dsu.size(1) == 3);
    CHECK(dsu.components() == 3);
    CHECK(dsu.groups().size() == 3);
}

void test_persistent_data_structures() {
    const cp::PersistentArray<int> original(std::vector{1, 2, 3});
    const auto changed = original.set(1, 9);
    CHECK(original.get(1) == 2);
    CHECK(changed.get(1) == 9);

    const cp::PersistentDisjointSet base(4);
    const auto version1 = base.merge(0, 1);
    const auto version2 = version1.merge(2, 3);
    CHECK(!base.same(0, 1));
    CHECK(version1.same(0, 1));
    CHECK(!version1.same(2, 3));
    CHECK(version2.same(2, 3));
    CHECK(version2.components() == 2);
}

void test_range_structures() {
    cp::FenwickTree<long long> fenwick(6);
    fenwick.add(0, 2);
    fenwick.add(3, 5);
    fenwick.add(5, 1);
    CHECK(fenwick.prefix_sum(4) == 7);
    CHECK(fenwick.sum(1, 6) == 6);
    CHECK(fenwick.lower_bound(7) == 3);
    cp::MonoidFenwickTree<cp::MaxMonoid<int>> prefix_max(6);
    prefix_max.update(1, 4);
    prefix_max.update(3, 9);
    CHECK(prefix_max.prefix_fold(3) == 4);
    CHECK(prefix_max.prefix_fold(4) == 9);

    cp::OrderedMultiset ordered(10);
    ordered.insert(2);
    ordered.insert(5, 2);
    CHECK(ordered.order_of_key(5) == 1);
    CHECK(ordered.find_by_order(1) == 5);
    CHECK(ordered.size() == 3);

    cp::SegmentTree<cp::SumMonoid<long long>> segment(std::vector<long long>{1, 2, 3, 4, 5});
    CHECK(segment.fold(1, 4) == 9);
    segment.set(2, 10);
    CHECK(segment.all_fold() == 22);
    CHECK(segment.max_right(0, [](long long sum) { return sum <= 7; }) == 2);
    CHECK(segment.min_left(5, [](long long sum) { return sum <= 9; }) == 3);

    cp::LazySegmentTree<cp::RangeAddRangeSum<long long>> lazy(std::vector<long long>{1, 2, 3, 4, 5});
    lazy.apply(1, 4, 10);
    CHECK(lazy.fold(0, 5) == 45);
    CHECK(lazy.get(2) == 13);
    lazy.set(2, 0);
    CHECK(lazy.fold(1, 4) == 26);

    cp::SparseTable<cp::MinMonoid<int>> sparse(std::vector{5, 2, 7, 1, 3});
    CHECK(sparse.fold(1, 4) == 1);

    const std::vector<cp::MinMaxPositionMonoid<int>> minmax_data{
        cp::MinMaxPositionMonoid<int>::point(3, 0),
        cp::MinMaxPositionMonoid<int>::point(1, 1),
        cp::MinMaxPositionMonoid<int>::point(4, 2),
        cp::MinMaxPositionMonoid<int>::point(2, 3),
        cp::MinMaxPositionMonoid<int>::point(5, 4),
    };
    cp::SegmentTree<cp::MinMaxPositionMonoid<int>> minmax_tree(minmax_data);
    const auto minmax_fold = minmax_tree.fold(0, 3);
    CHECK(minmax_fold.mmin == 1 && minmax_fold.pmin == 1);
    CHECK(minmax_fold.mmax == 4 && minmax_fold.pmax == 2);
    CHECK(minmax_fold.min_value() == 1 && minmax_fold.min_index() == 1);
    CHECK(minmax_fold.max_value() == 4 && minmax_fold.max_index() == 2);

    cp::SegmentTree<cp::MinIndexMonoid<int>> min_idx_tree(std::vector<cp::MinIndexMonoid<int>>{
        cp::MinIndexMonoid<int>::point(10, 0),
        cp::MinIndexMonoid<int>::point(5, 1),
        cp::MinIndexMonoid<int>::point(8, 2),
    });
    CHECK(min_idx_tree.fold(0, 3).value == 5);
    CHECK(min_idx_tree.fold(0, 3).index == 1);

    cp::SegmentTree<cp::MaxIndexMonoid<int>> max_idx_tree(std::vector<cp::MaxIndexMonoid<int>>{
        cp::MaxIndexMonoid<int>::point(10, 0),
        cp::MaxIndexMonoid<int>::point(15, 1),
        cp::MaxIndexMonoid<int>::point(8, 2),
    });
    CHECK(max_idx_tree.fold(0, 3).value == 15);
    CHECK(max_idx_tree.fold(0, 3).index == 1);
}

void test_strings() {
    cp::Trie trie;
    trie.insert("app");
    trie.insert("apple");
    trie.insert("app");
    CHECK(trie.contains("apple"));
    CHECK(!trie.contains("ap"));
    CHECK(trie.count("app") == 2);
    CHECK(trie.count_prefix("app") == 3);

    CHECK((cp::z_function(std::string_view("aabcaabxaaaz")) == std::vector{12, 1, 0, 0, 3, 1, 0, 0, 2, 2, 1, 0}));
    CHECK((cp::prefix_function(std::string_view("ababcabab")) == std::vector{0, 0, 1, 2, 0, 1, 2, 3, 4}));

    const cp::RollingHash hash("abracadabra");
    CHECK(hash.hash(0, 4) == hash.hash(7, 11));
    CHECK(hash.hash(0, 3) != hash.hash(3, 6));
}

void test_graphs() {
    cp::Graph<int> graph(5);
    graph.add_directed_edge(0, 1, 2);
    graph.add_directed_edge(0, 2, 8);
    graph.add_directed_edge(1, 2, 1);
    graph.add_directed_edge(2, 3, 3);
    const auto shortest = cp::dijkstra(graph, 0);
    CHECK(shortest.distance[3] == 6);
    CHECK((shortest.path_to(3) == std::vector{0, 1, 2, 3}));
    CHECK(shortest.path_to(4).empty());

    const auto topology = cp::topological_sort(graph);
    CHECK(topology.has_value());
    CHECK(topology->front() == 0);
    CHECK(cp::bfs_distance(graph, 0)[3] == 2);
    CHECK((cp::dfs_preorder(graph, 0) == std::vector{0, 1, 2, 3}));

    cp::Graph<int> binary_graph(3);
    binary_graph.add_directed_edge(0, 1, 1);
    binary_graph.add_directed_edge(0, 2, 0);
    binary_graph.add_directed_edge(2, 1, 0);
    CHECK(cp::zero_one_bfs(binary_graph, 0).distance[1] == 0);

    cp::Graph<int> negative(3);
    negative.add_directed_edge(0, 1, 1);
    negative.add_directed_edge(1, 2, -3);
    CHECK(cp::bellman_ford(negative, 0)->distance[2] == -2);
    negative.add_directed_edge(2, 1, 1);
    CHECK(!cp::bellman_ford(negative, 0).has_value());

    cp::Graph<int> scc_graph(4);
    scc_graph.add_directed_edge(0, 1);
    scc_graph.add_directed_edge(1, 0);
    scc_graph.add_directed_edge(1, 2);
    scc_graph.add_directed_edge(2, 3);
    scc_graph.add_directed_edge(3, 2);
    const auto scc = cp::strongly_connected_components(scc_graph);
    CHECK(scc.count == 2);
    CHECK(scc.component[0] == scc.component[1]);
    CHECK(scc.component[1] != scc.component[2]);

    cp::Graph<long long> tree(5);
    tree.add_edge(0, 1, 2);
    tree.add_edge(1, 2, 3);
    tree.add_edge(1, 3, 4);
    tree.add_edge(3, 4, 5);
    const cp::LowestCommonAncestor lca(tree);
    CHECK(lca.lca(2, 4) == 1);
    CHECK(lca.edge_distance(2, 4) == 3);
    CHECK(lca.weighted_distance(2, 4) == 12);
    const auto [diameter, path] = cp::tree_diameter(tree);
    CHECK(diameter == 12);
    CHECK(path.front() == 4 && path.back() == 2);
    CHECK((cp::tree_centers(tree) == std::vector{1, 3}));
    const auto mst = cp::kruskal(tree);
    CHECK(mst.has_value() && mst->weight == 14 && mst->edges.size() == 4);

    cp::BipartiteMatching matching(3, 3);
    matching.add_edge(0, 0);
    matching.add_edge(0, 1);
    matching.add_edge(1, 1);
    matching.add_edge(2, 1);
    matching.add_edge(2, 2);
    CHECK(matching.maximum_matching() == 3);

    cp::Dinic<int> flow(4);
    flow.add_edge(0, 1, 3);
    flow.add_edge(0, 2, 2);
    flow.add_edge(1, 2, 1);
    flow.add_edge(1, 3, 2);
    flow.add_edge(2, 3, 3);
    CHECK(flow.max_flow(0, 3) == 5);
    const auto cut = flow.min_cut(0);
    CHECK(cut[0] && !cut[3]);

    cp::StringVertices names;
    CHECK(names.get_or_add("alpha") == 0);
    CHECK(names.get_or_add("beta") == 1);
    CHECK(names.get_or_add("alpha") == 0);
    CHECK(names.name(1) == "beta");

    int neighbors = 0;
    cp::for_each_grid_neighbor(cp::GridPoint{0, 0}, 3, 4, [&](cp::GridPoint) { ++neighbors; });
    CHECK(neighbors == 2);
}

void test_tree_structures() {
    // 1. RootedTree & dsu_on_tree
    cp::Graph<long long> tree(6);
    tree.add_edge(0, 1, 10);
    tree.add_edge(0, 2, 20);
    tree.add_edge(1, 3, 30);
    tree.add_edge(1, 4, 40);
    tree.add_edge(2, 5, 50);

    cp::RootedTree<long long> rtree(tree, 0);
    CHECK(rtree.parent(0) == -1);
    CHECK(rtree.parent(3) == 1);
    CHECK(rtree.depth(3) == 2);
    CHECK(rtree.subtree_size(1) == 3);
    CHECK(rtree.subtree_size(0) == 6);
    CHECK(rtree.is_ancestor(1, 3));
    CHECK(!rtree.is_ancestor(3, 1));
    CHECK(!rtree.is_ancestor(1, 5));
    CHECK(rtree.lca(3, 4) == 1);
    CHECK(rtree.lca(3, 5) == 0);
    CHECK(rtree.kth_ancestor(3, 1) == 1);
    CHECK(rtree.kth_ancestor(3, 2) == 0);
    CHECK(rtree.edge_distance(3, 5) == 4);
    CHECK(rtree.weighted_distance(3, 5) == 110);
    CHECK((rtree.path(3, 5) == std::vector{3, 1, 0, 2, 5}));

    // Test dsu_on_tree
    std::vector<int> colors{1, 2, 1, 2, 2, 3};
    std::vector<int> freq(4, 0);
    std::vector<int> distinct_colors(6, 0);
    int distinct_count = 0;
    cp::dsu_on_tree(
        rtree,
        [&](int v) {
            if (++freq[colors[v]] == 1) ++distinct_count;
        },
        [&](int v) {
            if (--freq[colors[v]] == 0) --distinct_count;
        },
        [&](int v) {
            distinct_colors[v] = distinct_count;
        }
    );
    CHECK(distinct_colors[3] == 1);
    CHECK(distinct_colors[1] == 1);
    CHECK(distinct_colors[2] == 2);
    CHECK(distinct_colors[0] == 3);

    // 2. HeavyLightDecomposition
    cp::HeavyLightDecomposition hld(tree, 0);
    CHECK(hld.lca(3, 4) == 1);
    CHECK(hld.lca(3, 5) == 0);
    CHECK(hld.dist(3, 5) == 4);
    CHECK(hld.kth_ancestor(4, 2) == 0);

    std::vector<int> path_vertices;
    hld.for_each_path_interval(3, 5, [&](int l, int r) {
        for (int i = l; i < r; ++i) {
            path_vertices.push_back(hld.vertex_at(i));
        }
    });
    std::sort(path_vertices.begin(), path_vertices.end());
    CHECK((path_vertices == std::vector{0, 1, 2, 3, 5}));

    // 3. CentroidDecomposition
    cp::CentroidDecomposition cd(tree);
    CHECK(cd.size() == 6);
    int cd_root = cd.root();
    CHECK(cd_root >= 0 && cd_root < 6);
    CHECK(cd.parent(cd_root) == -1);
    for (int i = 0; i < 6; ++i) {
        CHECK(cd.level(i) <= 3);
        int cur = i;
        int count = 0;
        cd.for_each_ancestor(i, [&](int ancestor) {
            CHECK(ancestor == cur);
            cur = cd.parent(cur);
            ++count;
        });
        CHECK(cur == -1);
        CHECK(count == cd.level(i) + 1);
    }

    // 4. DFS Tree (Undirected & Directed)
    cp::Graph<int> ugraph(4);
    ugraph.add_edge(0, 1);
    ugraph.add_edge(1, 2);
    ugraph.add_edge(2, 0);
    ugraph.add_edge(0, 3);
    const auto udfs = cp::build_dfs_tree_undirected(ugraph);
    CHECK(udfs.depth.size() == 4);
    CHECK(udfs.parent.size() == 4);
    CHECK(udfs.span_edges.size() == 3);
    CHECK(udfs.back_edges.size() == 1);
    CHECK(udfs.bridges.size() == 1);
    CHECK((udfs.bridges[0] == std::pair{0, 3} || udfs.bridges[0] == std::pair{3, 0}));
    CHECK(udfs.two_edge_connected_count == 2);
    CHECK(udfs.two_edge_connected_components[0] == udfs.two_edge_connected_components[1]);
    CHECK(udfs.two_edge_connected_components[1] == udfs.two_edge_connected_components[2]);
    CHECK(udfs.two_edge_connected_components[0] != udfs.two_edge_connected_components[3]);
    const auto bottom_up = udfs.order_by_depth_desc();
    CHECK(bottom_up.size() == 4);
    CHECK(udfs.depth[bottom_up.front()] >= udfs.depth[bottom_up.back()]);

    const auto simple_dfs = cp::build_simple_dfs_tree(ugraph, 0);
    CHECK(simple_dfs.depth.size() == 4);
    CHECK(simple_dfs.parent.size() == 4);
    CHECK(simple_dfs.span_edges.size() == 3);
    CHECK(simple_dfs.back_edges.size() == 1);
    CHECK(simple_dfs.depth[0] == 0);
    CHECK(simple_dfs.back_edges[0].first == 0);

    cp::Graph<int> dgraph(4);
    dgraph.add_directed_edge(0, 1);
    dgraph.add_directed_edge(1, 2);
    dgraph.add_directed_edge(2, 0);
    dgraph.add_directed_edge(0, 3);
    dgraph.add_directed_edge(3, 2);
    const auto ddfs = cp::build_dfs_tree_directed(dgraph);
    CHECK(!ddfs.back_edges.empty());
    CHECK(!ddfs.cross_edges.empty());

    // Deep graph test to verify iterative DFS avoids stack overflow on deep paths
    const int deep_n = 100000;
    cp::Graph<int> deep_graph(deep_n);
    for (int i = 0; i + 1 < deep_n; ++i) {
        deep_graph.add_edge(i, i + 1);
    }
    const auto deep_dfs = cp::build_dfs_tree_undirected(deep_graph, 0);
    CHECK(deep_dfs.depth[deep_n - 1] == deep_n - 1);
    CHECK(deep_dfs.bridges.size() == static_cast<std::size_t>(deep_n - 1));

    // 5. RingGraph (Pseudotree)
    cp::Graph<int> ring_g(5);
    ring_g.add_edge(0, 1);
    ring_g.add_edge(1, 2);
    ring_g.add_edge(2, 0);
    ring_g.add_edge(1, 3);
    ring_g.add_edge(3, 4);
    cp::RingGraph ring(ring_g);
    CHECK(ring.cycle_length() == 3);
    CHECK(ring.is_on_cycle(0) && ring.is_on_cycle(1) && ring.is_on_cycle(2));
    CHECK(!ring.is_on_cycle(3) && !ring.is_on_cycle(4));
    CHECK(ring.root_on_cycle(4) == 1);
    CHECK(ring.distance_to_cycle(4) == 2);
    CHECK(ring.cycle_distance(0, 2) == 1);
    CHECK((ring.path(4, 0) == std::vector{4, 3, 1, 0}));
}

void test_modular_arithmetic() {
    using Mint = cp::ModInt998244353;
    CHECK(Mint(-1).value() == 998244352U);
    CHECK((Mint(10) / Mint(2)).value() == 5U);
    CHECK(Mint(3).pow(5).value() == 243U);

    cp::Combinatorics<Mint> combinatorics(10);
    CHECK(combinatorics.binomial(10, 3).value() == 120U);
    CHECK(combinatorics.permutation(5, 2).value() == 20U);
    CHECK(combinatorics.multiset(3, 4).value() == 15U);
    CHECK(cp::binomial_u64(10, 4) == 210);
    CHECK(cp::weak_compositions(3, 4) == 15);
    CHECK(cp::compositions(3, 5) == 6);
    CHECK(cp::non_consecutive_selections(5, 3) == 1);
    CHECK(cp::binomial_mod_lucas(100, 20, 13) == 5);
    std::size_t cycle_type_count = 0;
    cp::enumerate_cycle_types<Mint>(4, combinatorics,
        [&](cp::u128, Mint, Mint) { ++cycle_type_count; });
    CHECK(cycle_type_count == 5);

    const std::vector permutation{1, 2, 0, 4, 3};
    const auto split = cp::split_cycles(permutation);
    CHECK(split.cycles.size() == 2);
    CHECK((cp::permutation_power(permutation, 2) == std::vector{2, 0, 1, 3, 4}));
}

void test_number_theory() {
    const auto egcd = cp::extended_gcd(30, 18);
    CHECK(egcd.gcd == 6);
    CHECK(30 * egcd.x + 18 * egcd.y == 6);
    CHECK(cp::mod_inverse(3, 11) == 4);
    CHECK(!cp::mod_inverse(6, 9).has_value());
    CHECK(cp::power_mod(2, 10, 1000) == 24);
    CHECK(cp::factorial_prime_valuation(10, 2) == 8);
    CHECK(cp::floor_sum(4, 10, 6, 3) == 3);
    CHECK(cp::floor_sum(3, 5, -2, 6) == 1);

    const auto ranges = cp::floor_quotient_ranges(20);
    CHECK(ranges.size() == 8);
    CHECK(ranges.back().left == 11 && ranges.back().right == 20 && ranges.back().quotient == 1);
    CHECK((cp::mobius_sieve(10) == std::vector{0, 1, -1, -1, 0, -1, 1, -1, 0, 0, 1}));
    CHECK((cp::totient_sieve(10) == std::vector<std::uint64_t>{0, 1, 1, 2, 2, 4, 2, 6, 4, 6, 4}));

    const std::vector<cp::PrimePowerFactor> factors{{2, 3}, {3, 2}, {5, 1}};
    auto divisors = cp::divisors_from_factors(factors);
    std::sort(divisors.begin(), divisors.end());
    CHECK(divisors.size() == 24);
    CHECK(divisors.front() == 1 && divisors.back() == 360);
    CHECK(cp::square_free_divisors_from_factors(factors).size() == 8);
    CHECK((cp::chinese_remainder(2, 3, 3, 5) == std::pair<std::int64_t, std::int64_t>{8, 15}));
    CHECK(cp::baby_step_giant_step(2, 8, 13) == 3);
    CHECK(cp::multiplicative_order_prime(3, 7, {{2, 1}, {3, 1}}) == 6);
    CHECK(cp::count_coprime_pairs_up_to(5, cp::mobius_sieve(5)) == 9);
    const auto repeated = cp::RadixBlock::from_integer(12, 10, 1'000'000).repeat(3, 1'000'000);
    CHECK(repeated.value == 121212);
    CHECK(cp::multiplicative_order(10, 63) == 6);
    const cp::WeightedPrefixInterval interval{8, 3};
    const auto pieces = interval.apply_mod(5);
    CHECK(pieces.size() == 2);
    CHECK(pieces[0].end == 5 && pieces[0].copies == 3);
    CHECK(pieces[1].end == 3 && pieces[1].copies == 3);
}

void test_primes() {
    cp::PrimeSieve sieve(100);
    CHECK(sieve.is_prime(97));
    CHECK(!sieve.is_prime(1));
    CHECK(sieve.smallest_prime_factor(84) == 2);
    CHECK(sieve.prime_at_most(20) == 19);
    CHECK(sieve.prime_at_least(20) == 23);
    CHECK((sieve.factorize(72) == std::vector<cp::PrimePowerFactor>{{2, 3}, {3, 2}}));
    CHECK(cp::is_prime(1'000'000'007));
    CHECK(!cp::is_prime(1'000'000'005));
    const auto root = cp::primitive_root_prime(17);
    CHECK(cp::power_mod(root, 8, 17) != 1);
}

void test_polynomials_and_matrix() {
    const auto floating = cp::convolution(std::vector<double>{1, 2, 3}, std::vector<double>{4, 5});
    CHECK(floating.size() == 4);
    CHECK(std::abs(floating[0] - 4.0) < 1e-8);
    CHECK(std::abs(floating[2] - 22.0) < 1e-8);
    CHECK((cp::convolution_ll({1, 2, 3}, {4, 5}) == std::vector<long long>{4, 13, 22, 15}));

    using Mint = cp::ModInt998244353;
    const auto modular = cp::convolution(std::vector<Mint>{1, 2, 3}, std::vector<Mint>{4, 5});
    CHECK((modular == std::vector<Mint>{4, 13, 22, 15}));

    const cp::Fps998244353 series{1, 1};
    CHECK((series.pow(3, 5).coefficients() == std::vector<Mint>{1, 3, 3, 1, 0}));
    const cp::Fps998244353 logarithm_input{1, 1};
    const auto recovered = logarithm_input.logarithm(6).exponential(6);
    CHECK(recovered[0] == Mint{1} && recovered[1] == Mint{1});
    CHECK(recovered[2] == Mint{} && recovered[5] == Mint{});
    const auto square_root = cp::Fps998244353{1, 2, 1}.square_root(3);
    CHECK(square_root.has_value());
    CHECK((square_root->coefficients() == std::vector<Mint>{1, 1, 0}));
    const auto pairwise = cp::Fps998244353::sum_pairwise_convolution(
        {cp::Fps998244353{1, 1}, cp::Fps998244353{1, 2}, cp::Fps998244353{2}}, 3);
    CHECK((pairwise.coefficients() == std::vector<Mint>{5, 9, 2}));

    std::vector<Mint> large_left(40);
    std::vector<Mint> large_right(40);
    for (std::size_t index = 0; index < 40; ++index) {
        large_left[index] = Mint(index + 1);
        large_right[index] = Mint(40 - index);
    }
    const auto large_product = cp::convolution(large_left, large_right);
    Mint coefficient_39 = 0;
    for (std::size_t index = 0; index <= 39; ++index) {
        coefficient_39 += large_left[index] * large_right[39 - index];
    }
    CHECK(large_product[39] == coefficient_39);

    cp::Matrix<Mint, 2, 2> fibonacci({{{Mint{1}, Mint{1}}, {Mint{1}, Mint{0}}}});
    const auto powered = fibonacci.pow(10);
    CHECK(powered[0][1] == Mint{55});
    CHECK((fibonacci * std::array<Mint, 2>{Mint{1}, Mint{0}} == std::array<Mint, 2>{Mint{1}, Mint{1}}));
}

}  // namespace

int main() {
    test_utils();
    test_recursive_memoization();
    test_basic_data_structures();
    test_persistent_data_structures();
    test_range_structures();
    test_strings();
    test_graphs();
    test_tree_structures();
    test_modular_arithmetic();
    test_number_theory();
    test_primes();
    test_polynomials_and_matrix();

    if (failures != 0) {
        std::cerr << failures << " checks failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "cp_library_test: all checks passed\n";
}

