#include <raze/algorithm/find/Equal.h>

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
void test_equal_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i);
        auto vec2 = vec1;
        
        auto simd_result = raze::algorithm::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        if (size == 0) continue;
        auto vec1 = generate_random_vector<T>(size, seed + i + 1000);
        auto vec2 = vec1;
        
        std::uniform_int_distribution<size_t> pos_dist(0, size - 1);
        size_t pos = pos_dist(size_gen);
        vec2[pos] = (vec1[pos] == T(0)) ? T(1) : T(0);
        
        auto simd_result = raze::algorithm::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
}

template <typename T>
void test_equal_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 2000);
        auto vec2 = vec1;
        
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        if (size == 0) continue;
        auto vec1 = generate_random_vector<T>(size, seed + i + 3000);
        auto vec2 = vec1;
        
        std::uniform_int_distribution<size_t> pos_dist(0, size - 1);
        size_t pos = pos_dist(size_gen);
        vec2[pos] = (vec1[pos] == T(0)) ? T(1) : T(0);
        
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
}

template <typename T>
void test_equal_pred_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 4000);
        auto vec2 = vec1;
        
        auto pred = [](const T& a, const T& b) { return a == b; };
        
        auto simd_result = raze::algorithm::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred);
        auto std_result = std::ranges::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 5000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 6000);
        
        auto pred_true = [](const T&, const T&) { return true; };
        
        auto simd_result = raze::algorithm::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_true);
        auto std_result = std::ranges::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_true);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        if (size == 0) continue;
        auto vec1 = generate_random_vector<T>(size, seed + i + 7000);
        auto vec2 = vec1;
        
        auto pred_false = [](const T&, const T&) { return false; };
        
        auto simd_result = raze::algorithm::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_false);
        auto std_result = std::ranges::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_false);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
}

template <typename T>
void test_equal_pred_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 8000);
        auto vec2 = vec1;
        
        auto pred = [](const T& a, const T& b) { return a == b; };
        
        auto simd_result = raze::algorithm::equal(vec1, vec2, pred);
        auto std_result = std::ranges::equal(vec1, vec2, pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_equal_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec1(100);
        std::vector<Point> vec2(100);
        RandomGenerator<int> gen(seed + i + 9000);
        for (size_t j = 0; j < 100; ++j) {
            vec1[j].x = gen();
            vec1[j].y = gen();
            vec2[j].x = vec1[j].x;
            vec2[j].y = gen();
        }
        
        auto proj = [](const Point& p) { return p.x; };
        auto pred = [](int a, int b) { return a == b; };
        
        auto simd_result = raze::algorithm::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred, proj, proj);
        auto std_result = std::ranges::equal(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred, proj, proj);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
        
        vec2[50].x = vec1[50].x + 1;
        
        auto simd_result2 = raze::algorithm::equal(vec1, vec2, pred, proj, proj);
        auto std_result2 = std::ranges::equal(vec1, vec2, pred, proj, proj);
        
        raze_assert(simd_result2 == std_result2);
        raze_assert(simd_result2 == false);
    }
}

template <typename T>
void test_equal_edge_cases() {
    {
        std::vector<T> vec1;
        std::vector<T> vec2;
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        raze_assert(simd_result == true);
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec1 = {T(42)};
        std::vector<T> vec2;
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        raze_assert(simd_result == false);
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2)};
        std::vector<T> vec2 = {T(1), T(2), T(3)};
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        raze_assert(simd_result == false);
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec1 = {T(42)};
        std::vector<T> vec2 = {T(42)};
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        raze_assert(simd_result == true);
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec1 = {T(42)};
        std::vector<T> vec2 = {T(43)};
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        raze_assert(simd_result == false);
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec1(1000, T(42));
        std::vector<T> vec2(1000, T(42));
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        raze_assert(simd_result == true);
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec1(1000, T(42));
        std::vector<T> vec2(1000, T(42));
        vec2[999] = T(43);
        auto simd_result = raze::algorithm::equal(vec1, vec2);
        auto std_result = std::ranges::equal(vec1, vec2);
        raze_assert(simd_result == false);
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_equal_random<T>();
    test_equal_ranges<T>();
    test_equal_pred_random<T>();
    test_equal_pred_ranges<T>();
    test_equal_edge_cases<T>();
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
    
    test_equal_with_projection<int>();
    
    return 0;
}