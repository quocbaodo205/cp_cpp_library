#pragma once

#include <algorithm>
#include <limits>
#include <numeric>

namespace cp {

template <class T>
struct SumMonoid {
    using value_type = T;
    static constexpr T identity() { return T{}; }
    static constexpr T combine(const T& left, const T& right) { return left + right; }
};

template <class T>
struct ProductMonoid {
    using value_type = T;
    static constexpr T identity() { return T{1}; }
    static constexpr T combine(const T& left, const T& right) { return left * right; }
};

template <class T>
struct MinMonoid {
    using value_type = T;
    static constexpr T identity() { return std::numeric_limits<T>::max(); }
    static constexpr T combine(const T& left, const T& right) { return std::min(left, right); }
};

template <class T>
struct MaxMonoid {
    using value_type = T;
    static constexpr T identity() { return std::numeric_limits<T>::lowest(); }
    static constexpr T combine(const T& left, const T& right) { return std::max(left, right); }
};

template <class T>
struct GcdMonoid {
    using value_type = T;
    static constexpr T identity() { return T{}; }
    static constexpr T combine(const T& left, const T& right) { return std::gcd(left, right); }
};

template <class T>
struct BitAndMonoid {
    using value_type = T;
    static constexpr T identity() { return ~T{}; }
    static constexpr T combine(const T& left, const T& right) { return left & right; }
};

template <class T>
struct BitOrMonoid {
    using value_type = T;
    static constexpr T identity() { return T{}; }
    static constexpr T combine(const T& left, const T& right) { return left | right; }
};

template <class T>
struct BitXorMonoid {
    using value_type = T;
    static constexpr T identity() { return T{}; }
    static constexpr T combine(const T& left, const T& right) { return left ^ right; }
};

template <class T = int, class Index = int>
struct MinIndexMonoid {
    T value = std::numeric_limits<T>::max();
    Index index = Index{};

    using value_type = MinIndexMonoid;

    static constexpr MinIndexMonoid point(const T& val, Index idx) {
        return MinIndexMonoid{val, idx};
    }
    static constexpr MinIndexMonoid identity() {
        return MinIndexMonoid{};
    }
    static constexpr MinIndexMonoid combine(const MinIndexMonoid& left, const MinIndexMonoid& right) {
        if (right.value < left.value) {
            return right;
        }
        return left;
    }

    bool operator==(const MinIndexMonoid&) const = default;
};

template <class T = int, class Index = int>
struct MaxIndexMonoid {
    T value = std::numeric_limits<T>::lowest();
    Index index = Index{};

    using value_type = MaxIndexMonoid;

    static constexpr MaxIndexMonoid point(const T& val, Index idx) {
        return MaxIndexMonoid{val, idx};
    }
    static constexpr MaxIndexMonoid identity() {
        return MaxIndexMonoid{};
    }
    static constexpr MaxIndexMonoid combine(const MaxIndexMonoid& left, const MaxIndexMonoid& right) {
        if (right.value > left.value) {
            return right;
        }
        return left;
    }

    bool operator==(const MaxIndexMonoid&) const = default;
};

template <class T = int, class Index = int>
struct MinMaxPositionMonoid {
    T mmin = std::numeric_limits<T>::max();
    T mmax = std::numeric_limits<T>::lowest();
    Index pmin = Index{};
    Index pmax = Index{};

    using value_type = MinMaxPositionMonoid;

    static constexpr MinMaxPositionMonoid point(const T& value, Index index) {
        return MinMaxPositionMonoid{
            .mmin = value,
            .mmax = value,
            .pmin = index,
            .pmax = index,
        };
    }

    static constexpr MinMaxPositionMonoid identity() {
        return MinMaxPositionMonoid{};
    }

    static constexpr MinMaxPositionMonoid combine(const MinMaxPositionMonoid& left,
                                                  const MinMaxPositionMonoid& right) {
        MinMaxPositionMonoid res{};
        if (right.mmin < left.mmin) {
            res.mmin = right.mmin;
            res.pmin = right.pmin;
        } else {
            res.mmin = left.mmin;
            res.pmin = left.pmin;
        }

        if (right.mmax > left.mmax) {
            res.mmax = right.mmax;
            res.pmax = right.pmax;
        } else {
            res.mmax = left.mmax;
            res.pmax = left.pmax;
        }
        return res;
    }

    constexpr const T& min_value() const { return mmin; }
    constexpr const T& max_value() const { return mmax; }
    constexpr Index min_index() const { return pmin; }
    constexpr Index max_index() const { return pmax; }

    bool operator==(const MinMaxPositionMonoid&) const = default;
};

template <class T = int, class Index = int>
using MinMaxIndexMonoid = MinMaxPositionMonoid<T, Index>;

}  // namespace cp

