#pragma once

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace cp {

namespace detail {

inline void hash_combine(std::size_t& seed, std::size_t value) {
    seed ^= value + 0x9e3779b9U + (seed << 6U) + (seed >> 2U);
}

struct TupleHash {
    template <class... Values>
    std::size_t operator()(const std::tuple<Values...>& values) const {
        std::size_t seed = 0;
        std::apply(
            [&](const auto&... value) {
                (hash_combine(seed, std::hash<std::remove_cvref_t<decltype(value)>>{}(value)), ...);
            },
            values);
        return seed;
    }
};

template <class Function, class Hash, class Signature>
class MemoizedRecursive;

template <class Function, class Hash, class Return, class... Args>
class MemoizedRecursive<Function, Hash, Return(Args...)> {
    static_assert(!std::is_void_v<Return>);
    static_assert(!std::is_reference_v<Return>);

public:
    using key_type = std::tuple<std::decay_t<Args>...>;
    using result_type = Return;

    explicit MemoizedRecursive(Function function, Hash hash = {})
        : function_(std::move(function)), cache_(0, std::move(hash)) {}

    Return operator()(Args... args) {
        key_type key(args...);
        if (const auto iterator = cache_.find(key); iterator != cache_.end()) {
            return iterator->second;
        }
        Return result = std::invoke(function_, *this, args...);
        return cache_.try_emplace(std::move(key), std::move(result)).first->second;
    }

    bool contains(const Args&... args) const { return cache_.contains(key_type(args...)); }
    std::size_t size() const { return cache_.size(); }
    void reserve(std::size_t count) { cache_.reserve(count); }
    void clear() { cache_.clear(); }

private:
    Function function_;
    std::unordered_map<key_type, Return, Hash> cache_;
};

}  // namespace detail

template <class Signature, class Function, class Hash = detail::TupleHash>
auto memoize_recursive(Function&& function, Hash hash = {}) {
    return detail::MemoizedRecursive<std::decay_t<Function>, Hash, Signature>(
        std::forward<Function>(function), std::move(hash));
}

}  // namespace cp

