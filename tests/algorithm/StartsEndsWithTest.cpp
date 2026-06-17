#include <raze/algorithm/find/Find.h>
#include <raze/algorithm/find/AnyOf.h>
#include <raze/algorithm/find/NoneOf.h>
#include <raze/algorithm/find/AllOf.h>
#include <raze/algorithm/find/StartsWith.h>
#include <raze/algorithm/find/EndsWith.h>

#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <ranges>
#include <cassert>
#include <functional>

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
            } else {
                std::uniform_int_distribution<DistType> dist(0, 2000);
                return static_cast<T>(dist(gen));
            }
        } else if constexpr (std::is_floating_point_v<T>) {
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
std::vector<T> generate_vector_with_target(size_t size, T target, size_t target_pos, unsigned seed = 42) {
    auto vec = generate_random_vector<T>(size, seed);
    if (target_pos < size) {
        vec[target_pos] = target;
    }
    return vec;
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_find_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 120000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        
        int target_x = gen();
        
        auto simd_result = raze::algorithm::find_if(vec.begin(), vec.end(), 
            [target_x](const Point& p) { return p.x == target_x; },
            [](const Point& p) { return p; });
        
        auto std_result = std::find_if(vec.begin(), vec.end(), 
            [target_x](const Point& p) { return p.x == target_x; });
        
        raze_assert(simd_result == std_result);
        if (simd_result != vec.end()) {
            raze_assert(*simd_result == *std_result);
        }
    }
}

template <typename T>
void test_starts_with_random(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        size_t s1 = 50 + (i % 100);
        size_t s2 = 20 + (i % 50);
        if (i % 5 == 0) s2 = s1 + 10;
        if (i % 7 == 0) s2 = s1;
        
        auto vec1 = generate_random_vector<T>(s1, seed + i + 500000);
        auto vec2 = generate_random_vector<T>(s2, seed + i + 500000);
        
        auto simd_result = raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::starts_with(vec1, vec2);
        raze_assert(simd_result == std_result);

        auto simd_result_r = raze::algorithm::starts_with(vec1, vec2);
        auto std_result_r = std::ranges::starts_with(vec1, vec2);
        raze_assert(simd_result_r == std_result_r);
    }
}

template <typename T>
void test_ends_with_random(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        size_t s1 = 50 + (i % 100);
        size_t s2 = 20 + (i % 50);
        if (i % 5 == 0) s2 = s1 + 10;
        if (i % 7 == 0) s2 = s1;
        
        auto vec1 = generate_random_vector<T>(s1, seed + i + 520000);
        auto vec2 = generate_random_vector<T>(s2, seed + i + 520000);
        
        auto simd_result = raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::ends_with(vec1, vec2);
        raze_assert(simd_result == std_result);

        auto simd_result_r = raze::algorithm::ends_with(vec1, vec2);
        auto std_result_r = std::ranges::ends_with(vec1, vec2);
        raze_assert(simd_result_r == std_result_r);
    }
}

template <typename T>
void test_starts_ends_with_pred(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        size_t s1 = 50 + (i % 100);
        size_t s2 = 20 + (i % 50);
        if (i % 5 == 0) s2 = s1 + 10;
        
        auto vec1 = generate_random_vector<T>(s1, seed + i + 540000);
        auto vec2 = generate_random_vector<T>(s2, seed + i + 540000);
        
        auto pred = [](T a, T b) { return a == b; };
        
        auto simd_result_sw = raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred);
        auto std_result_sw = std::ranges::starts_with(vec1, vec2, pred);
        raze_assert(simd_result_sw == std_result_sw);

        auto simd_result_ew = raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred);
        auto std_result_ew = std::ranges::ends_with(vec1, vec2, pred);
        raze_assert(simd_result_ew == std_result_ew);
    }
}

template <typename T>
void test_starts_with_edge_cases() {
    {
        std::vector<T> vec1;
        std::vector<T> vec2;
        raze_assert(raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::starts_with(vec1, vec2));
        raze_assert(raze::algorithm::starts_with(vec1, vec2) == std::ranges::starts_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2)};
        std::vector<T> vec2;
        raze_assert(raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::starts_with(vec1, vec2));
        raze_assert(raze::algorithm::starts_with(vec1, vec2) == std::ranges::starts_with(vec1, vec2));
    }
    {
        std::vector<T> vec1;
        std::vector<T> vec2 = {T(1)};
        raze_assert(raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::starts_with(vec1, vec2));
        raze_assert(raze::algorithm::starts_with(vec1, vec2) == std::ranges::starts_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(1), T(2), T(3), T(4)};
        raze_assert(raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::starts_with(vec1, vec2));
        raze_assert(raze::algorithm::starts_with(vec1, vec2) == std::ranges::starts_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(1), T(2)};
        raze_assert(raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::starts_with(vec1, vec2));
        raze_assert(raze::algorithm::starts_with(vec1, vec2) == std::ranges::starts_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(1), T(3)};
        raze_assert(raze::algorithm::starts_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::starts_with(vec1, vec2));
        raze_assert(raze::algorithm::starts_with(vec1, vec2) == std::ranges::starts_with(vec1, vec2));
    }
}

template <typename T>
void test_ends_with_edge_cases() {
    {
        std::vector<T> vec1;
        std::vector<T> vec2;
        raze_assert(raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::ends_with(vec1, vec2));
        raze_assert(raze::algorithm::ends_with(vec1, vec2) == std::ranges::ends_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2)};
        std::vector<T> vec2;
        raze_assert(raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::ends_with(vec1, vec2));
        raze_assert(raze::algorithm::ends_with(vec1, vec2) == std::ranges::ends_with(vec1, vec2));
    }
    {
        std::vector<T> vec1;
        std::vector<T> vec2 = {T(1)};
        raze_assert(raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::ends_with(vec1, vec2));
        raze_assert(raze::algorithm::ends_with(vec1, vec2) == std::ranges::ends_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(0), T(1), T(2), T(3)};
        raze_assert(raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::ends_with(vec1, vec2));
        raze_assert(raze::algorithm::ends_with(vec1, vec2) == std::ranges::ends_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(2), T(3)};
        raze_assert(raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::ends_with(vec1, vec2));
        raze_assert(raze::algorithm::ends_with(vec1, vec2) == std::ranges::ends_with(vec1, vec2));
    }
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(1), T(3)};
        raze_assert(raze::algorithm::ends_with(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()) == std::ranges::ends_with(vec1, vec2));
        raze_assert(raze::algorithm::ends_with(vec1, vec2) == std::ranges::ends_with(vec1, vec2));
    }
}

template <typename T>
void test_starts_ends_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        std::vector<Point> vec1(100);
        std::vector<Point> vec2(50);
        RandomGenerator<int> gen(seed + i + 130000);
        for (auto& p : vec1) {
            p.x = gen();
            p.y = gen();
        }
        for (auto& p : vec2) {
            p.x = gen();
            p.y = gen();
        }
        
        auto proj = [](const Point& p) { return p.x; };
        
        auto simd_result_sw = raze::algorithm::starts_with(vec1, vec2, std::equal_to<>{}, proj, proj);
        auto std_result_sw = std::ranges::starts_with(vec1, vec2, std::equal_to<>{}, proj, proj);
        raze_assert(simd_result_sw == std_result_sw);
        
        auto simd_result_ew = raze::algorithm::ends_with(vec1, vec2, std::equal_to<>{}, proj, proj);
        auto std_result_ew = std::ranges::ends_with(vec1, vec2, std::equal_to<>{}, proj, proj);
        raze_assert(simd_result_ew == std_result_ew);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_starts_with_random<T>();
    test_ends_with_random<T>();
    test_starts_ends_with_pred<T>();
    test_starts_with_edge_cases<T>();
    test_ends_with_edge_cases<T>();
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
    
    test_starts_ends_with_projection<int>();
    
    return 0;
}