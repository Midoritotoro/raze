#include <raze/algorithm/find/Find.h>
#include <raze/algorithm/find/AnyOf.h>
#include <raze/algorithm/find/NoneOf.h>
#include <raze/algorithm/find/AllOf.h>

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
void test_any_of_random(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 200000);
        T threshold = RandomGenerator<T>(seed + i + 200000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::any_of(vec.begin(), vec.end(), pred);
        auto std_result = std::any_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 210000);
        auto pred = [](T) { return false; };
        
        auto simd_result = raze::algorithm::any_of(vec.begin(), vec.end(), pred);
        auto std_result = std::any_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 220000);
        auto pred = [](T) { return true; };
        
        auto simd_result = raze::algorithm::any_of(vec.begin(), vec.end(), pred);
        auto std_result = std::any_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
}

template <typename T>
void test_any_of_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 230000);
        T threshold = RandomGenerator<T>(seed + i + 230000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::any_of(vec, pred);
        auto std_result = std::ranges::any_of(vec, pred);
        
        raze_assert(simd_result == std_result);
    }
}

// ==================== ТЕСТЫ ДЛЯ none_of ====================

template <typename T>
void test_none_of_random(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 300000);
        T threshold = RandomGenerator<T>(seed + i + 300000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::none_of(vec.begin(), vec.end(), pred);
        auto std_result = std::none_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 310000);
        auto pred = [](T) { return false; };
        
        auto simd_result = raze::algorithm::none_of(vec.begin(), vec.end(), pred);
        auto std_result = std::none_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 320000);
        auto pred = [](T) { return true; };
        
        auto simd_result = raze::algorithm::none_of(vec.begin(), vec.end(), pred);
        auto std_result = std::none_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
}

template <typename T>
void test_none_of_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 330000);
        T threshold = RandomGenerator<T>(seed + i + 330000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::none_of(vec, pred);
        auto std_result = std::ranges::none_of(vec, pred);
        
        raze_assert(simd_result == std_result);
    }
}

// ==================== ТЕСТЫ ДЛЯ all_of ====================

template <typename T>
void test_all_of_random(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 400000);
        T threshold = RandomGenerator<T>(seed + i + 400000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::all_of(vec.begin(), vec.end(), pred);
        auto std_result = std::all_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 410000);
        auto pred = [](T) { return false; };
        
        auto simd_result = raze::algorithm::all_of(vec.begin(), vec.end(), pred);
        auto std_result = std::all_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 420000);
        auto pred = [](T) { return true; };
        
        auto simd_result = raze::algorithm::all_of(vec.begin(), vec.end(), pred);
        auto std_result = std::all_of(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
}

template <typename T>
void test_all_of_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 430000);
        T threshold = RandomGenerator<T>(seed + i + 430000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::all_of(vec, pred);
        auto std_result = std::ranges::all_of(vec, pred);
        
        raze_assert(simd_result == std_result);
    }
}

// ==================== ГРАНИЧНЫЕ СЛУЧАИ ====================

template <typename T>
void test_any_of_edge_cases() {
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::any_of(vec.begin(), vec.end(), [](T) { return true; });
        auto std_result = std::any_of(vec.begin(), vec.end(), [](T) { return true; });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::any_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::any_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec(100, T(0));
        auto simd_result = raze::algorithm::any_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::any_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
    
    {
        std::vector<T> vec(100, T(42));
        auto simd_result = raze::algorithm::any_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::any_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
}

template <typename T>
void test_none_of_edge_cases() {
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::none_of(vec.begin(), vec.end(), [](T) { return true; });
        auto std_result = std::none_of(vec.begin(), vec.end(), [](T) { return true; });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::none_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::none_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
    
    {
        std::vector<T> vec(100, T(0));
        auto simd_result = raze::algorithm::none_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::none_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec(100, T(42));
        auto simd_result = raze::algorithm::none_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::none_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
}

template <typename T>
void test_all_of_edge_cases() {
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::all_of(vec.begin(), vec.end(), [](T) { return false; });
        auto std_result = std::all_of(vec.begin(), vec.end(), [](T) { return false; });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::all_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::all_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
    
    {
        std::vector<T> vec(100, T(0));
        auto simd_result = raze::algorithm::all_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::all_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == false);
    }
    
    {
        std::vector<T> vec(100, T(42));
        auto simd_result = raze::algorithm::all_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        auto std_result = std::all_of(vec.begin(), vec.end(), [](T x) { return x == T(42); });
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == true);
    }
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
void run_all_tests_for_type() {
    test_any_of_random<T>();
    test_any_of_ranges<T>();
    test_any_of_edge_cases<T>();
    
    test_none_of_random<T>();
    test_none_of_ranges<T>();
    test_none_of_edge_cases<T>();
    
    test_all_of_random<T>();
    test_all_of_ranges<T>();
    test_all_of_edge_cases<T>();
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
    
    test_find_with_projection<int>();
    
    return 0;
}