#include <raze/algorithm/find/SearchN.h>

#include <vector>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <ranges>

template <typename T>
struct IntDistributionType {
    using type = T;
};

template <> struct IntDistributionType<char> { using type = int; };
template <> struct IntDistributionType<signed char> { using type = int; };
template <> struct IntDistributionType<unsigned char> { using type = int; };
template <> struct IntDistributionType<bool> { using type = int; };
template <> struct IntDistributionType<wchar_t> { using type = int; };
template <> struct IntDistributionType<char8_t> { using type = int; };
template <> struct IntDistributionType<char16_t> { using type = int; };
template <> struct IntDistributionType<char32_t> { using type = int; };

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
void test_search_n_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    std::uniform_int_distribution<int> count_dist(0, 50);
    
    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        T target = gen();
        int count = count_dist(size_gen);
        
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), count, target);
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), count, target);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
    
    for (size_t pos : {0, 1, 7, 15, 31, 63, 127, 255, 511, 990}) {
        for (int count = 1; count <= 10; ++count) {
            if (pos + count <= 1000) {
                auto vec = generate_random_vector<T>(1000, seed + pos + count);
                for (int j = 0; j < count; ++j) {
                    vec[pos + j] = T(42);
                }
                
                if (pos == 31 && count == 10)
                    pos = 31;

                auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), count, T(42));
                auto std_result = std::ranges::search_n(vec.begin(), vec.end(), count, T(42));
                raze_assert(simd_result.begin() == std_result.begin());
                raze_assert(simd_result.end() == std_result.end());
            }
        }
    }
}

template <typename T>
void test_search_n_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    std::uniform_int_distribution<int> count_dist(0, 10);

    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        T target = RandomGenerator<T>(seed + i + 30000)();
        int count = count_dist(size_gen);
        
        auto simd_result = raze::algorithm::search_n(vec, count, target);
        auto std_result = std::ranges::search_n(vec, count, target);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
}

template <typename T>
void test_search_n_edge_cases() {
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), 0, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), 0, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.begin());
    }
    
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), 5, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), 5, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.end());
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(3)};
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), -5, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), -5, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.begin());
    }
    
    {
        std::vector<T> vec = {T(42), T(42), T(42)};
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), 5, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), 5, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.end());
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(42), T(3)};
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), 1, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), 1, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.begin() + 2);
    }
    
    {
        std::vector<T> vec(10, T(42));
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), 10, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), 10, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.begin());
    }
    
    {
        std::vector<T> vec(10, T(42));
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), 5, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), 5, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.begin());
    }
    
    {
        std::vector<T> vec(10, T(0));
        vec[7] = T(42); vec[8] = T(42); vec[9] = T(42);
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), 3, T(42));
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), 3, T(42));
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.begin() + 7);
    }
}

template <typename T>
void test_search_n_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        for (int count = 1; count <= 5; ++count) {
            if (static_cast<size_t>(count) <= size) {
                std::vector<T> vec(size, T(0));
                size_t pos = size - count;
                for (int i = 0; i < count; ++i) vec[pos + i] = T(42);
                
                auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), count, T(42));
                auto std_result = std::ranges::search_n(vec.begin(), vec.end(), count, T(42));
                
                raze_assert(simd_result.begin() == std_result.begin());
                raze_assert(simd_result.begin() == vec.begin() + pos);
            }
        }
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

void test_search_n_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 10000; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 120000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        
        int count = 3;
        size_t pos = 50;
        for (int j = 0; j < count; ++j) {
            vec[pos + j].x = 42;
        }
        
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), count, 42, 
            std::ranges::equal_to{}, [](const Point& p) { return p.x; });
        
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), count, 42, 
            std::ranges::equal_to{}, [](const Point& p) { return p.x; });
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
}

template <typename T>
void test_search_n_custom_pred(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<T> vec(100, T(0));
        int count = 3;
        size_t pos = 40;
        for (int j = 0; j < count; ++j) {
            vec[pos + j] = T(100);
        }
        
        auto pred = [](T elem, T val) { return elem >= val; };
        T target = T(50);
        
        auto simd_result = raze::algorithm::search_n(vec.begin(), vec.end(), count, target, pred);
        auto std_result = std::ranges::search_n(vec.begin(), vec.end(), count, target, pred);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.begin() == vec.begin() + pos);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_search_n_random<T>();
    test_search_n_ranges<T>();
    test_search_n_edge_cases<T>();
    test_search_n_simd_boundaries<T>();
    test_search_n_custom_pred<T>();
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
    
    test_search_n_with_projection();
    
    return 0;
}