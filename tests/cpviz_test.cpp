#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cpviz/graph.hpp>
#include <cpviz/grid.hpp>
#include <cpviz/string.hpp>

#include "cp/graph/graph.hpp"

namespace {

int failures = 0;

void check(bool condition, const char *expression, const char *test, int line) {
    if (!condition) {
        ++failures;
        std::cerr << test << ':' << line << ": check failed: " << expression << '\n';
    }
}

#define CHECK(expression) check(static_cast<bool>(expression), #expression, __func__, __LINE__)

void test_trace_serialization() {
    cpviz::ArrayTrace trace("quotes: \" and newline\n");
    trace.display_index_base(1);
    trace.frame(std::vector{3, 1, 4}, "initial")
        .pointer("i", 1)
        .range(0, 2, "prefix")
        .arrow(0, 2, "dependency")
        .mark(1, cpviz::Tone::active)
        .field(1, "kind", "pivot")
        .watch("answer", 4);

    std::ostringstream output;
    trace.write_json(output);
    const std::string json = output.str();
    CHECK(json.contains("\"schema\": \"cpviz.array-trace\""));
    CHECK(json.contains("\"display_index_base\": 1"));
    CHECK(json.contains("quotes: \\\" and newline\\n"));
    CHECK(json.contains("\"tone\":\"active\""));
    CHECK(json.contains("\"label\":\"dependency\""));
    CHECK(json.contains("\"left_closed\":true,\"right_closed\":false"));
    CHECK(json.contains("\"name\":\"answer\",\"value\":\"4\""));
}

void test_custom_formatter() {
    struct State {
        int cost;
        int ways;
    };

    cpviz::ArrayTrace trace("custom values");
    trace.frame(std::vector<State>{{7, 2}}, "formatted", [](const State &state) {
        auto result = cpviz::cell(state.cost);
        result.field("ways", state.ways);
        return result;
    });

    std::ostringstream output;
    trace.write_json(output);
    CHECK(output.str().contains("\"text\":\"7\""));
    CHECK(output.str().contains("\"name\":\"ways\",\"value\":\"2\""));
}

void test_aligned_value_row() {
    cpviz::ArrayTrace trace("rows");
    auto &frame = trace.frame(std::vector{4, 2, 1}, "array and prefix sums");
    auto &row = frame.row("prefix", std::vector{4, 6, 7});
    row.mark(1, cpviz::Tone::active).field(1, "source", "first two values");

    std::ostringstream output;
    trace.write_json(output);
    CHECK(output.str().contains("\"view\": \"array\""));
    CHECK(output.str().contains("\"rows\":[{\"label\":\"prefix\""));

    bool length_threw = false;
    try {
        frame.row("too short", std::vector{1, 2});
    } catch (const std::invalid_argument &) {
        length_threw = true;
    }
    CHECK(length_threw);
}

void test_string_trace() {
    cpviz::StringTrace trace("changes");
    trace.frame("abc", "before");
    auto &changed = trace.frame("axc", "after");
    changed.substring(0, 2, "prefix").row("z", std::vector{3, 0, 0}).mark(1, cpviz::Tone::active);

    CHECK(changed.cells()[0].tone() == cpviz::Tone::neutral);
    CHECK(changed.cells()[1].tone() == cpviz::Tone::active);
    CHECK(changed.cells()[1].fields()[0].name == "previous");
    CHECK(changed.cells()[1].fields()[0].value == "b");
    CHECK(changed.ranges().size() == 1);
    CHECK(changed.rows().size() == 1);

    std::ostringstream output;
    trace.write_json(output);
    CHECK(output.str().contains("\"view\": \"string\""));
    CHECK(output.str().contains("\"changed positions\",\"value\":\"1\""));
}

void test_graph_trace() {
    cp::Graph<int> graph(3);
    const int first_edge = graph.add_edge(0, 1, 7);
    graph.add_edge(1, 2, 4);

    cpviz::GraphTrace trace(graph, "graph", {.directed = false, .show_edge_weights = true});
    trace.node_label(0, "root");
    auto &frame = trace.frame("tree");
    frame.tree_layout(0, std::vector{-1, 0, 1})
        .mark_node(0, cpviz::Tone::active)
        .node_value(1, 42)
        .node_field(1, "depth", 1)
        .style_edge(first_edge, cpviz::EdgeKind::tree, cpviz::Tone::focus)
        .watch("visited", 3);

    CHECK(frame.nodes().size() == 3);
    CHECK(frame.edges().size() == 2);
    CHECK(frame.nodes()[0].label == "root");
    CHECK(frame.edges()[0].label == "7");
    CHECK(frame.parents() == std::vector<int>({-1, 0, 1}));

    std::ostringstream output;
    trace.write_json(output);
    CHECK(output.str().contains("\"schema\": \"cpviz.graph-trace\""));
    CHECK(output.str().contains("\"layout\":\"tree\""));
    CHECK(output.str().contains("\"kind\":\"tree\""));

    bool parents_threw = false;
    try {
        frame.tree_layout(0, std::vector{-1, 0});
    } catch (const std::invalid_argument &) {
        parents_threw = true;
    }
    CHECK(parents_threw);

    cp::Graph<int> directed_graph(2);
    directed_graph.add_directed_edge(0, 1, 5);
    directed_graph.add_directed_edge(1, 0, 8);
    cpviz::GraphTrace directed_trace(directed_graph, "directed", {.directed = true, .show_edge_weights = true});
    directed_trace.frame("both directions");
    CHECK(directed_trace.frames()[0].edges().size() == 2);
    std::ostringstream directed_output;
    directed_trace.write_json(directed_output);
    CHECK(directed_output.str().contains("\"directed\": true"));
}

void test_grid_trace() {
    cpviz::GridTrace trace("grid");
    trace.display_index_base(1);
    auto &frame = trace.frame(std::vector<std::string>{"#.", ".#"}, "cells");
    frame.mark(0, 0, cpviz::Tone::blocked)
        .mark_rect(1, 0, 2, 2, cpviz::Tone::focus)
        .field(1, 1, "kind", "target")
        .watch("rows", 2);

    CHECK(frame.rows() == 2);
    CHECK(frame.columns() == 2);
    CHECK(frame.cells().size() == 4);
    CHECK(frame.cells()[0].tone() == cpviz::Tone::blocked);
    CHECK(frame.cells()[3].fields()[0].value == "target");

    std::ostringstream output;
    trace.write_json(output);
    CHECK(output.str().contains("\"schema\": \"cpviz.grid-trace\""));
    CHECK(output.str().contains("\"row_index_base\": 1"));
    CHECK(output.str().contains("\"rows\":2,\"columns\":2"));

    bool ragged_threw = false;
    try {
        trace.frame(std::vector<std::vector<int>>{{1, 2}, {3}}, "ragged");
    } catch (const std::invalid_argument &) {
        ragged_threw = true;
    }
    CHECK(ragged_threw);
}

void test_invalid_annotations() {
    cpviz::ArrayTrace trace("bounds");
    auto &frame = trace.frame(std::vector{1, 2}, "initial");

    bool mark_threw = false;
    try {
        frame.mark(2, cpviz::Tone::active);
    } catch (const std::out_of_range &) {
        mark_threw = true;
    }
    CHECK(mark_threw);

    bool range_threw = false;
    try {
        frame.range(0, 3);
    } catch (const std::out_of_range &) {
        range_threw = true;
    }
    CHECK(range_threw);
}

} // namespace

int main() {
    test_trace_serialization();
    test_custom_formatter();
    test_aligned_value_row();
    test_string_trace();
    test_graph_trace();
    test_grid_trace();
    test_invalid_annotations();
    return failures == 0 ? 0 : 1;
}
