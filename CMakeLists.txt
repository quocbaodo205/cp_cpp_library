cmake_minimum_required(VERSION 3.25)

project(cp_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CP_SOURCE "solutions/main.cpp" CACHE FILEPATH "Solution source to build and bundle")
cmake_path(ABSOLUTE_PATH CP_SOURCE
    BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    NORMALIZE
    OUTPUT_VARIABLE CP_SOURCE_ABS
)

if(NOT EXISTS "${CP_SOURCE_ABS}")
    message(FATAL_ERROR "CP_SOURCE does not exist: ${CP_SOURCE_ABS}")
endif()

add_library(cp_library INTERFACE)
target_include_directories(cp_library INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(cp_library INTERFACE cxx_std_23)

# Local visualization helpers are deliberately separate from cp_library so they
# never enter a normal bundled submission.
add_library(cpviz INTERFACE)
target_include_directories(cpviz INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/include")
target_compile_features(cpviz INTERFACE cxx_std_23)

option(CP_ENABLE_VIZ "Enable CP_VIZ blocks in the selected local solution" OFF)

add_executable(cp_main "${CP_SOURCE_ABS}")
target_link_libraries(cp_main PRIVATE cp_library)
if(CP_ENABLE_VIZ)
    target_link_libraries(cp_main PRIVATE cpviz)
    target_compile_definitions(cp_main PRIVATE CP_VIZ=1)
endif()
target_compile_options(cp_main PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Competitive-programming code uses int vertex/state indices with std::vector.
    target_compile_options(cp_main PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32)
endif()

find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)

add_executable(cpviz_array_demo EXCLUDE_FROM_ALL playground/array_viz.cpp)
target_link_libraries(cpviz_array_demo PRIVATE cpviz)
target_compile_options(cpviz_array_demo PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
    -Werror
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(cpviz_array_demo PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32)
endif()

set(CPVIZ_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/viz")
set(CPVIZ_ARRAY_TRACE "${CPVIZ_OUTPUT_DIR}/array_trace.json")
set(CPVIZ_ARRAY_HTML "${CPVIZ_OUTPUT_DIR}/array_trace.html")
add_custom_target(array_viz
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CPVIZ_OUTPUT_DIR}"
    COMMAND "$<TARGET_FILE:cpviz_array_demo>" "${CPVIZ_ARRAY_TRACE}"
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/render.py"
        "${CPVIZ_ARRAY_TRACE}"
        --output "${CPVIZ_ARRAY_HTML}"
    BYPRODUCTS "${CPVIZ_ARRAY_TRACE}" "${CPVIZ_ARRAY_HTML}"
    DEPENDS
        cpviz_array_demo
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/render.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/viewer/array.html"
    COMMENT "Generating the interactive array visualization"
    VERBATIM
)

add_executable(cpviz_string_demo EXCLUDE_FROM_ALL playground/string_viz.cpp)
target_link_libraries(cpviz_string_demo PRIVATE cp_library cpviz)
target_compile_options(cpviz_string_demo PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
    -Werror
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(cpviz_string_demo PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32)
endif()

set(CPVIZ_STRING_CHANGES_TRACE "${CPVIZ_OUTPUT_DIR}/string_changes.json")
set(CPVIZ_STRING_CHANGES_HTML "${CPVIZ_OUTPUT_DIR}/string_changes.html")
set(CPVIZ_Z_FUNCTION_TRACE "${CPVIZ_OUTPUT_DIR}/z_function.json")
set(CPVIZ_Z_FUNCTION_HTML "${CPVIZ_OUTPUT_DIR}/z_function.html")
add_custom_target(string_viz
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CPVIZ_OUTPUT_DIR}"
    COMMAND "$<TARGET_FILE:cpviz_string_demo>" "${CPVIZ_OUTPUT_DIR}"
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/render.py"
        "${CPVIZ_STRING_CHANGES_TRACE}"
        --output "${CPVIZ_STRING_CHANGES_HTML}"
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/render.py"
        "${CPVIZ_Z_FUNCTION_TRACE}"
        --output "${CPVIZ_Z_FUNCTION_HTML}"
    BYPRODUCTS
        "${CPVIZ_STRING_CHANGES_TRACE}"
        "${CPVIZ_STRING_CHANGES_HTML}"
        "${CPVIZ_Z_FUNCTION_TRACE}"
        "${CPVIZ_Z_FUNCTION_HTML}"
    DEPENDS
        cpviz_string_demo
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/render.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/viewer/array.html"
    COMMENT "Generating the interactive string visualizations"
    VERBATIM
)

add_executable(cpviz_graph_demo EXCLUDE_FROM_ALL playground/graph_viz.cpp)
target_link_libraries(cpviz_graph_demo PRIVATE cp_library cpviz)
target_compile_options(cpviz_graph_demo PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
    -Werror
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(cpviz_graph_demo PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32)
endif()

set(CPVIZ_GRAPH_TRACE "${CPVIZ_OUTPUT_DIR}/graph_trace.json")
set(CPVIZ_GRAPH_HTML "${CPVIZ_OUTPUT_DIR}/graph_trace.html")
add_custom_target(graph_viz
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CPVIZ_OUTPUT_DIR}"
    COMMAND "$<TARGET_FILE:cpviz_graph_demo>" "${CPVIZ_GRAPH_TRACE}"
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/render.py"
        "${CPVIZ_GRAPH_TRACE}"
        --output "${CPVIZ_GRAPH_HTML}"
    BYPRODUCTS "${CPVIZ_GRAPH_TRACE}" "${CPVIZ_GRAPH_HTML}"
    DEPENDS
        cpviz_graph_demo
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/render.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/cpviz/viewer/graph.html"
    COMMENT "Generating the interactive graph visualization"
    VERBATIM
)

file(GLOB_RECURSE CP_LIBRARY_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

set(CP_BUNDLED_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/submission.cpp")
add_custom_command(
    OUTPUT "${CP_BUNDLED_SOURCE}"
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/bundle.py"
        --source "${CP_SOURCE_ABS}"
        --include-dir "${CMAKE_CURRENT_SOURCE_DIR}/include"
        --strip-ifdef CP_VIZ
        --output "${CP_BUNDLED_SOURCE}"
    DEPENDS
        "${CP_SOURCE_ABS}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/bundle.py"
        ${CP_LIBRARY_HEADERS}
    COMMENT "Bundling ${CP_SOURCE} into submission.cpp"
    VERBATIM
)

add_custom_target(bundle DEPENDS "${CP_BUNDLED_SOURCE}")
add_executable(submission EXCLUDE_FROM_ALL "${CP_BUNDLED_SOURCE}")
target_compile_features(submission PRIVATE cxx_std_23)
target_compile_options(submission PRIVATE
    -Wall
    -Wextra
    -Wshadow
    -Wconversion
)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(submission PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(cp_library_test tests/cp_library_test.cpp)
    target_link_libraries(cp_library_test PRIVATE cp_library)
    target_compile_options(cp_library_test PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -Werror
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(cp_library_test PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32)
    endif()
    add_test(NAME cp_library_test COMMAND cp_library_test)

    add_executable(cpviz_test tests/cpviz_test.cpp)
    target_link_libraries(cpviz_test PRIVATE cp_library cpviz)
    target_compile_options(cpviz_test PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -Werror
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(cpviz_test PRIVATE -Wno-sign-conversion -Wno-shorten-64-to-32)
    endif()
    add_test(NAME cpviz_test COMMAND cpviz_test)

    add_test(
        NAME bundle_test
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/bundle_test.py"
    )
endif()

