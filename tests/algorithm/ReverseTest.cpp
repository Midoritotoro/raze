#include <raze/algorithm/order/Reverse.h>
#include <raze/algorithm/order/ReverseCopy.h>

#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>

template <typename T>
struct IntDistributionType {
    using type = T;
};

template <>
struct IntDistributionType<char> {
    using type = int;
};

template <>
struct IntDistributionType<signed char> {
    using type = int;
};

template <>
struct IntDistributionType<unsigned char> {
    using type = int;
};

template <>
struct IntDistributionType<bool> {
    using type = int;
};

template <>
struct IntDistributionType<wchar_t> {
    using type = int;
};

template <>
struct IntDistributionType<char8_t> {
    using type = int;
};

template <>
struct IntDistributionType<char16_t> {
    using type = int;
};

template <>
struct IntDistributionType<char32_t> {
    using type = int;
};

template <typename T>
using int_distribution_type_t = typename IntDistributionType<T>::type;

template <typename T>
struct RandomGenerator {
    std::mt19937 gen;

    RandomGenerator(unsigned seed = 42) : gen(seed) {}

    T operator()() {
        if constexpr (std::is_integral_v<T>) {
            using DistType = int_distribution_type_t<T>;

            if constexpr (std::is_signed_v<T>) {
                std::uniform_int_distribution<DistType> dist(-1000, 1000);
                return static_cast<T>(dist(gen));
            }
            else {
                std::uniform_int_distribution<DistType> dist(0, 2000);
                return static_cast<T>(dist(gen));
            }
        }
        else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(-1000.0, 1000.0);
            return dist(gen);
        }
    }
};

template <typename T>
std::vector<T> generate_random_vector(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::vector<T> vec(size);
    for (auto& elem : vec) {
        elem = gen();
    }
    return vec;
}

template <typename T>
void test_reverse_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_reverse_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        auto vec_copy = vec;

        raze::algorithm::reverse(vec);
        std::ranges::reverse(vec_copy);

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_reverse_copy_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 60000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto simd_result = raze::algorithm::reverse_copy(vec.begin(), vec.end(), simd_out.begin());
        auto std_result = std::ranges::reverse_copy(vec.begin(), vec.end(), std_out.begin());

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
        raze_assert(simd_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
    }
}

template <typename T>
void test_reverse_copy_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 90000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        auto simd_result = raze::algorithm::reverse_copy(vec, simd_out.begin());
        auto std_result = std::ranges::reverse_copy(vec, std_out.begin());

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
        raze_assert(simd_result.in == vec.end());
        raze_assert(simd_result.out == simd_out.end());
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_reverse_edge_cases() {
    {
        std::vector<T> vec;
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
    }

    {
        std::vector<T> vec = { T(42) };
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec[0] == T(42));
    }

    {
        std::vector<T> vec = { T(1), T(2) };
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec[0] == T(2));
        raze_assert(vec[1] == T(1));
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3) };
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec[0] == T(3));
        raze_assert(vec[1] == T(2));
        raze_assert(vec[2] == T(1));
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec[0] == T(5));
        raze_assert(vec[1] == T(4));
        raze_assert(vec[2] == T(3));
        raze_assert(vec[3] == T(2));
        raze_assert(vec[4] == T(1));
    }

    {
        std::vector<T> vec(100, T(42));
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
        for (const auto& x : vec) {
            raze_assert(x == T(42));
        }
    }

    {
        std::vector<T> vec;
        auto vec_copy = vec;
        std::vector<T> simd_out;
        std::vector<T> std_out;

        raze::algorithm::reverse_copy(vec.begin(), vec.end(), simd_out.begin());
        std::ranges::reverse_copy(vec_copy.begin(), vec_copy.end(), std_out.begin());

        raze_assert(simd_out == std_out);
    }

    {
        std::vector<T> vec = { T(1) };
        std::vector<T> simd_out(1);
        std::vector<T> std_out(1);

        raze::algorithm::reverse_copy(vec.begin(), vec.end(), simd_out.begin());
        std::ranges::reverse_copy(vec.begin(), vec.end(), std_out.begin());

        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(1));
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3) };
        std::vector<T> simd_out(3);
        std::vector<T> std_out(3);

        raze::algorithm::reverse_copy(vec.begin(), vec.end(), simd_out.begin());
        std::ranges::reverse_copy(vec.begin(), vec.end(), std_out.begin());

        raze_assert(simd_out == std_out);
        raze_assert(simd_out[0] == T(3));
        raze_assert(simd_out[1] == T(2));
        raze_assert(simd_out[2] == T(1));
        raze_assert(vec[0] == T(1));
    }
}

template <typename T>
void test_reverse_return_value() {
    {
        std::vector<T> vec = { T(1), T(2), T(3) };
        std::vector<T> out(3);

        auto result = raze::algorithm::reverse_copy(vec.begin(), vec.end(), out.begin());

        raze_assert(result.in == vec.end());
        raze_assert(result.out == out.end());
    }

    {
        std::vector<T> vec;
        std::vector<T> out;

        auto result = raze::algorithm::reverse_copy(vec.begin(), vec.end(), out.begin());

        raze_assert(result.in == vec.end());
        raze_assert(result.out == out.end());
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3) };
        std::vector<T> out(3);

        auto [in, out_it] = raze::algorithm::reverse_copy(vec.begin(), vec.end(), out.begin());

        raze_assert(in == vec.end());
        raze_assert(out_it == out.end());
    }
}

template <typename T>
void test_reverse_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 5000);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        raze::algorithm::reverse_copy(vec.begin(), vec.end(), simd_out.begin());
        std::ranges::reverse_copy(vec.begin(), vec.end(), std_out.begin());

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }
}

template <typename T>
void test_reverse_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 999);
        auto vec_original = vec;
        std::vector<T> simd_out(size);
        std::vector<T> std_out(size);

        raze::algorithm::reverse_copy(vec.begin(), vec.end(), simd_out.begin());
        std::ranges::reverse_copy(vec.begin(), vec.end(), std_out.begin());

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }
}

template <typename T>
void test_reverse_double_reverse(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 500);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 200000);
        auto vec_original = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        raze::algorithm::reverse(vec.begin(), vec.end());

        raze_assert(vec == vec_original);
    }
}

template <typename T>
void test_reverse_palindrome(unsigned seed = 42) {
    for (size_t size : {1, 3, 5, 7, 9, 15, 31, 63, 127}) {
        std::vector<T> vec(size);
        for (size_t i = 0; i < size; ++i) {
            vec[i] = static_cast<T>(i);
        }
        for (size_t i = 0; i < size / 2; ++i) {
            vec[size - 1 - i] = vec[i];
        }
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_reverse_copy_source_unchanged(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 500);

    for (int i = 0; i < 5000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 300000);
        auto vec_original = vec;
        std::vector<T> out(size);

        raze::algorithm::reverse_copy(vec.begin(), vec.end(), out.begin());

        raze_assert(vec == vec_original);
    }
}

template <typename T>
void test_reverse_struct(unsigned seed = 42) {
    for (int i = 0; i < 10000; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 400000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        auto vec_copy = vec;

        raze::algorithm::reverse(vec.begin(), vec.end());
        std::ranges::reverse(vec_copy.begin(), vec_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (int i = 0; i < 10000; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 500000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        auto vec_original = vec;
        std::vector<Point> simd_out(100);
        std::vector<Point> std_out(100);

        raze::algorithm::reverse_copy(vec.begin(), vec.end(), simd_out.begin());
        std::ranges::reverse_copy(vec.begin(), vec.end(), std_out.begin());

        raze_assert(simd_out == std_out);
        raze_assert(vec == vec_original);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_reverse_random<T>();
    test_reverse_ranges<T>();
    test_reverse_copy_random<T>();
    test_reverse_copy_ranges<T>();
    test_reverse_edge_cases<T>();
    test_reverse_return_value<T>();
    test_reverse_simd_boundaries<T>();
    test_reverse_large_vectors<T>();
    test_reverse_double_reverse<T>();
    test_reverse_palindrome<T>();
    test_reverse_copy_source_unchanged<T>();
    test_reverse_struct<T>();
}

int main() {
    run_all_tests_for_type<int>();
    run_all_tests_for_type<short>();
    run_all_tests_for_type<long long>();
    run_all_tests_for_type<char>();

    run_all_tests_for_type<unsigned int>();
    run_all_tests_for_type<unsigned short>();
    run_all_tests_for_type<unsigned long long>();
    run_all_tests_for_type<unsigned char>();

    run_all_tests_for_type<float>();
    run_all_tests_for_type<double>();

    test_reverse_struct<int>();

    return 0;
}