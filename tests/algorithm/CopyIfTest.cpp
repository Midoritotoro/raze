#include <raze/algorithm/copy/CopyIf.h>

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
void test_copy_if_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 40000);
        T threshold = RandomGenerator<T>(seed + i + 40000)();

        auto pred = [threshold](const T& x) { return x > threshold; };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 60000);
        T target = RandomGenerator<T>(seed + i + 60000)();

        auto pred = [target](const T& x) { return x == target; };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
    }

    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 80000);

        auto pred = [](const T&) { return false; };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.empty());
    }

    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 100000);

        auto pred = [](const T&) { return true; };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == size);
    }
}

template <typename T>
void test_copy_if_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 120000);
        T threshold = RandomGenerator<T>(seed + i + 120000)();

        auto pred = [threshold](const T& x) { return x > threshold; };

        std::vector<T> out_raze(size);
        auto result = raze::algorithm::copy_if(vec, out_raze.begin(), pred);
        out_raze.erase(result.second, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_copy_if_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 150000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }

        int threshold = gen();
        auto pred = [threshold](int x) { return x > threshold; };
        auto proj = [](const Point& p) { return p.x; };

        std::vector<Point> out_raze(100);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred, proj);
        out_raze.erase(out_end, out_raze.end());

        std::vector<Point> out_std(100);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred, proj);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
    }
}

template <typename T>
void test_copy_if_edge_cases() {
    {
        std::vector<T> vec;

        auto pred = [](const T&) { return true; };

        std::vector<T> out_raze;
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std;
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.empty());
    }

    {
        std::vector<T> vec = { T(42) };

        auto pred = [](const T&) { return true; };

        std::vector<T> out_raze(1);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(1);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == 1);
        raze_assert(out_raze[0] == T(42));
    }

    {
        std::vector<T> vec = { T(42) };

        auto pred = [](const T&) { return false; };

        std::vector<T> out_raze(1);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(1);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.empty());
    }

    {
        std::vector<T> vec(100, T(42));

        auto pred = [](const T&) { return true; };

        std::vector<T> out_raze(100);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(100);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == 100);
        for (const auto& x : out_raze) {
            raze_assert(x == T(42));
        }
    }

    {
        std::vector<T> vec(100, T(42));

        auto pred = [](const T&) { return false; };

        std::vector<T> out_raze(100);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(100);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.empty());
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(2), T(1), T(2), T(3) };

        auto pred = [](const T& x) { return x != T(2); };

        std::vector<T> out_raze(7);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(7);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == 4);
        raze_assert(out_raze[0] == T(1));
        raze_assert(out_raze[1] == T(3));
        raze_assert(out_raze[2] == T(1));
        raze_assert(out_raze[3] == T(3));
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };

        auto pred = [](const T& x) { return x == T(1); };

        std::vector<T> out_raze(5);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(5);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == 1);
        raze_assert(out_raze[0] == T(1));
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };

        auto pred = [](const T& x) { return x == T(5); };

        std::vector<T> out_raze(5);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(5);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == 1);
        raze_assert(out_raze[0] == T(5));
    }

    if constexpr (!std::is_floating_point_v<T>) {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };

        auto pred = [](const T& x) { return x % 2 != 0; };

        std::vector<T> out_raze(5);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(5);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == 3);
        raze_assert(out_raze[0] == T(1));
        raze_assert(out_raze[1] == T(3));
        raze_assert(out_raze[2] == T(5));
    }
}

template <typename T>
void test_copy_if_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        std::vector<T> vec(size, T(42));

        auto pred = [](const T&) { return true; };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == size);
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));

        T val = vec.empty() ? T(0) : vec[0];
        auto pred = [val](const T& x) { return x == val; };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 5000);

        auto pred = [](const T& x) { return x > T(0); };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
    }
}

template <typename T>
void test_copy_if_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 999);

        auto pred = [](const T& x) { return x > T(0); };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
    }
}

template <typename T>
void test_copy_if_all_same(unsigned seed = 42) {
    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));

        auto pred = [](const T& x) { return x == T(42); };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.size() == size);
    }

    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));

        auto pred = [](const T& x) { return x != T(42); };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.empty());
    }
}

template <typename T>
void test_copy_if_none_match(unsigned seed = 42) {
    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));

        auto pred = [](const T& x) { return x == T(99); };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);
        raze_assert(out_raze.empty());
    }
}

template <typename T>
void test_copy_if_mixed_pattern(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 500);

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 200000);

        T val = vec[size / 4];
        auto pred = [val](const T& x) { return x == val; };

        std::vector<T> out_raze(size);
        auto [in_end, out_end] = raze::algorithm::copy_if(vec.begin(), vec.end(), out_raze.begin(), pred);
        out_raze.erase(out_end, out_raze.end());

        std::vector<T> out_std(size);
        auto std_result = std::ranges::copy_if(vec, out_std.begin(), pred);
        out_std.erase(std_result.out, out_std.end());

        raze_assert(out_raze == out_std);

        for (const auto& x : out_raze) {
            raze_assert(x == val);
        }
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_copy_if_random<T>();
    test_copy_if_ranges<T>();
    test_copy_if_with_projection<T>();
    test_copy_if_edge_cases<T>();
    test_copy_if_simd_boundaries<T>();
    test_copy_if_large_vectors<T>();
    test_copy_if_all_same<T>();
    test_copy_if_none_match<T>();
    test_copy_if_mixed_pattern<T>();
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

    test_copy_if_with_projection<int>();

    return 0;
}