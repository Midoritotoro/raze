#include <raze/algorithm/find/Contains.h>

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

template <typename T>
void test_contains_random(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 500000);
        T target = RandomGenerator<T>(seed + i + 500000)();
        
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), target);
        auto std_result = (std::find(vec.begin(), vec.end(), target) != vec.end());
        
        raze_assert(simd_result == std_result);
    }
    
    for (size_t pos : {0, 1, 7, 15, 31, 63, 99}) {
        auto vec = generate_vector_with_target<T>(100, T(42), pos, seed);
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == true);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 510000);
        T missing_target = std::is_integral_v<T> ? static_cast<T>(999999) : static_cast<T>(999999.9);
        
        auto std_check = (std::find(vec.begin(), vec.end(), missing_target) != vec.end());
        if (!std_check) {
            auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), missing_target);
            raze_assert(simd_result == false);
        }
    }
}

template <typename T>
void test_contains_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 520000);
        T target = RandomGenerator<T>(seed + i + 520000)();
        
        auto simd_result = raze::algorithm::contains(vec, target);
        auto std_result = std::ranges::contains(vec, target);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_contains_edge_cases() {
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        auto std_result = (std::find(vec.begin(), vec.end(), T(42)) != vec.end());
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(99));
        raze_assert(simd_result == false);
    }
    
    {
        std::vector<T> vec(100, T(0));
        vec[0] = T(42);
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec(100, T(0));
        vec[99] = T(42);
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec(100, T(0));
        vec[50] = T(42);
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec(100, T(0));
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == false);
    }
    
    {
        std::vector<T> vec(100, T(42));
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == true);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_contains_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 10000; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 530000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        
        int target_x = gen();
        
        auto simd_result = raze::algorithm::contains(vec.begin(), vec.end(), target_x, [] (const Point& p) { return p.x; });
        auto std_result = (std::find_if(vec.begin(), vec.end(), [target_x](const Point& p) { return p.x == target_x; }) != vec.end());
        
        raze_assert(simd_result == std_result);
    }
    
    for (int i = 0; i < 10000; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 540000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        
        int target_x = gen();
        
        auto simd_result = raze::algorithm::contains(vec, target_x, [](const Point& p) { return p.x; });
        auto std_result = std::ranges::contains(vec, target_x, [](const Point& p) { return p.x; });
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_contains_random<T>();
    test_contains_ranges<T>();
    test_contains_edge_cases<T>();
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
    
    test_contains_with_projection<int>();
    
    return 0;
}