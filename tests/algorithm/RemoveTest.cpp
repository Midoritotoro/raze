#include <raze/algorithm/remove/Remove.h>

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
void test_remove_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        auto vec_copy = vec;
        T val = gen();

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), val);
        auto new_end_copy = std::ranges::remove(vec_copy, val);

        vec.erase(new_end.begin(), vec.end());
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 10000);
        auto vec_copy = vec;

        T val = vec.empty() ? T(0) : vec[size / 2];

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), val);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, val);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);

        for (const auto& x : vec) {
            raze_assert(x != val);
        }
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 20000);
        auto vec_copy = vec;

        T val = gen();

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), val);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, val);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_remove_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    RandomGenerator<T> gen(seed);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        auto vec_copy = vec;
        T val = gen();

        auto subrange = raze::algorithm::remove(vec, val);
        vec.erase(subrange.begin(), subrange.end());

        auto new_end_copy = std::ranges::remove(vec_copy, val);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_remove_if_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 40000);
        auto vec_copy = vec;
        T threshold = RandomGenerator<T>(seed + i + 40000)();

        auto pred = [threshold](const T& x) { return x > threshold; };

        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 60000);
        auto vec_copy = vec;
        T target = RandomGenerator<T>(seed + i + 60000)();

        auto pred = [target](const T& x) { return x == target; };

        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 80000);
        auto vec_copy = vec;

        auto pred = [](const T&) { return false; };

        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 100000);
        auto vec_copy = vec;

        auto pred = [](const T&) { return true; };

        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }
}

template <typename T>
void test_remove_if_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);

    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 120000);
        auto vec_copy = vec;
        T threshold = RandomGenerator<T>(seed + i + 120000)();

        auto pred = [threshold](const T& x) { return x > threshold; };

        auto subrange = raze::algorithm::remove_if(vec, pred);
        vec.erase(subrange.begin(), subrange.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_remove_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 140000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        auto vec_copy = vec;

        int target_x = gen();

        auto proj = [](const Point& p) { return p.x; };

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), target_x, proj);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, target_x, proj);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 150000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        auto vec_copy = vec;

        int threshold = gen();

        auto pred = [threshold](int x) { return x > threshold; };
        auto proj = [](const Point& p) { return p.x; };

        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred, proj);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred, proj);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_remove_edge_cases() {
    {
        std::vector<T> vec;
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(42));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(42));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }

    {
        std::vector<T> vec = { T(42) };
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(42));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(42));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }

    {
        std::vector<T> vec = { T(42) };
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(99));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(99));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == 1);
        raze_assert(vec[0] == T(42));
    }

    {
        std::vector<T> vec(100, T(42));
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(42));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(42));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }

    {
        std::vector<T> vec(100, T(42));
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(99));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(99));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == 100);
        for (const auto& x : vec) {
            raze_assert(x == T(42));
        }
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(2), T(1), T(2), T(3) };
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(2));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(2));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == 4);
        raze_assert(vec[0] == T(1));
        raze_assert(vec[1] == T(3));
        raze_assert(vec[2] == T(1));
        raze_assert(vec[3] == T(3));
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(1));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(1));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == 4);
        raze_assert(vec[0] == T(2));
    }

    {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(5));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(5));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == 4);
        raze_assert(vec[3] == T(4));
    }

    {
        std::vector<T> vec;
        auto vec_copy = vec;

        auto pred = [](const T&) { return true; };
        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }

    if constexpr (!std::is_floating_point_v<T>) {
        std::vector<T> vec = { T(1), T(2), T(3), T(4), T(5) };
        auto vec_copy = vec;

        auto pred = [](const T& x) { return x % 2 == 0; };
        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == 3);
        raze_assert(vec[0] == T(1));
        raze_assert(vec[1] == T(3));
        raze_assert(vec[2] == T(5));
    }
}

template <typename T>
void test_remove_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        std::vector<T> vec(size, T(42));
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(42));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(42));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec_copy = vec;

        T val = vec.empty() ? T(0) : vec[0];

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), val);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, val);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 5000);
        auto vec_copy = vec;

        auto pred = [](const T& x) { return x > T(0); };

        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_remove_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec_copy = vec;

        T val = vec.empty() ? T(0) : vec[size / 3];

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), val);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, val);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }

    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 999);
        auto vec_copy = vec;

        auto pred = [](const T& x) { return x > T(0); };

        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_remove_all_same(unsigned seed = 42) {
    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(42));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(42));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }

    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        auto vec_copy = vec;

        auto pred = [](const T& x) { return x == T(42); };
        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.empty());
    }
}

template <typename T>
void test_remove_none_match(unsigned seed = 42) {
    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        auto vec_copy = vec;

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), T(99));
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, T(99));
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == size);
        for (const auto& x : vec) {
            raze_assert(x == T(42));
        }
    }

    for (size_t size : {1, 10, 100, 1000}) {
        std::vector<T> vec(size, T(42));
        auto vec_copy = vec;

        auto pred = [](const T& x) { return x == T(99); };
        auto new_end = raze::algorithm::remove_if(vec.begin(), vec.end(), pred);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove_if(vec_copy, pred);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert(vec.size() == size);
        for (const auto& x : vec) {
            raze_assert(x == T(42));
        }
    }
}

template <typename T>
void test_remove_mixed_pattern(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 500);

    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 200000);
        auto vec_copy = vec;

        T val = vec[size / 4];

        auto new_end = raze::algorithm::remove(vec.begin(), vec.end(), val);
        vec.erase(new_end.begin(), vec.end());

        auto new_end_copy = std::ranges::remove(vec_copy, val);
        vec_copy.erase(new_end_copy.begin(), new_end_copy.end());

        raze_assert(vec == vec_copy);

        for (const auto& x : vec) {
            raze_assert(x != val);
        }
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_remove_random<T>();
    test_remove_ranges<T>();
    test_remove_if_random<T>();
    test_remove_if_ranges<T>();
    test_remove_with_projection<T>();
    test_remove_edge_cases<T>();
    test_remove_simd_boundaries<T>();
    test_remove_large_vectors<T>();
    test_remove_all_same<T>();
    test_remove_none_match<T>();
    test_remove_mixed_pattern<T>();
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

    test_remove_with_projection<int>();

    return 0;
}