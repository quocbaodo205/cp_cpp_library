#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace cp {

template <class T, std::size_t Rows, std::size_t Columns>
class Matrix {
public:
    constexpr Matrix() = default;
    constexpr explicit Matrix(std::array<std::array<T, Columns>, Rows> values) : values_(std::move(values)) {}

    constexpr std::array<T, Columns>& operator[](std::size_t row) { return values_[row]; }
    constexpr const std::array<T, Columns>& operator[](std::size_t row) const { return values_[row]; }

    constexpr auto operator<=>(const Matrix&) const = default;

    template <std::size_t OtherColumns>
    constexpr Matrix<T, Rows, OtherColumns> operator*(const Matrix<T, Columns, OtherColumns>& other) const {
        Matrix<T, Rows, OtherColumns> result;
        for (std::size_t row = 0; row < Rows; ++row) {
            for (std::size_t middle = 0; middle < Columns; ++middle) {
                for (std::size_t column = 0; column < OtherColumns; ++column) {
                    result[row][column] += values_[row][middle] * other[middle][column];
                }
            }
        }
        return result;
    }

    constexpr std::array<T, Rows> operator*(const std::array<T, Columns>& vector) const {
        std::array<T, Rows> result{};
        for (std::size_t row = 0; row < Rows; ++row) {
            for (std::size_t column = 0; column < Columns; ++column) {
                result[row] += values_[row][column] * vector[column];
            }
        }
        return result;
    }

    static constexpr Matrix identity() requires(Rows == Columns) {
        Matrix result;
        for (std::size_t index = 0; index < Rows; ++index) {
            result[index][index] = T{1};
        }
        return result;
    }

    constexpr Matrix pow(std::uint64_t exponent) const requires(Rows == Columns) {
        Matrix base = *this;
        Matrix result = identity();
        while (exponent > 0) {
            if ((exponent & 1U) != 0) {
                result = result * base;
            }
            base = base * base;
            exponent >>= 1U;
        }
        return result;
    }

private:
    std::array<std::array<T, Columns>, Rows> values_{};
};

}  // namespace cp

