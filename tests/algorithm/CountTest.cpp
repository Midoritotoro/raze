#include <raze/algorithm/find/Count.h>

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
void test_count_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        T target = gen();
        
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), target);
        auto std_result = std::ranges::count(vec.begin(), vec.end(), target);
        
        raze_assert(simd_result == std_result);
    }
    
    for (size_t pos : {0, 1, 7, 15, 31, 63, 127, 255, 511, 999}) {
        if (pos < 1000) {
            auto vec = generate_vector_with_target<T>(1000, T(42), pos, seed);
            auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(42));
            auto std_result = std::ranges::count(vec.begin(), vec.end(), T(42));
            raze_assert(simd_result == std_result);
            raze_assert(simd_result >= 1);
        }
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 20000);
        T target = gen();
        
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), target);
        auto std_result = std::ranges::count(vec.begin(), vec.end(), target);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_count_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 30000);
        T target = RandomGenerator<T>(seed + i + 30000)();
        
        auto simd_result = raze::algorithm::count(vec, target);
        auto std_result = std::ranges::count(vec, target);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_count_if_random(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 40000);
        T threshold = RandomGenerator<T>(seed + i + 40000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::count_if(vec.begin(), vec.end(), pred);
        auto std_result = std::ranges::count_if(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
    }
    
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 50000);
        T target = RandomGenerator<T>(seed + i + 50000)();
        
        auto pred = [target](T x) { return x == target; };
        
        auto simd_result = raze::algorithm::count_if(vec.begin(), vec.end(), pred);
        auto std_result = std::ranges::count_if(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 60000);
        auto pred = [](T) { return false; };
        
        auto simd_result = raze::algorithm::count_if(vec.begin(), vec.end(), pred);
        auto std_result = std::ranges::count_if(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 0);
    }
    
    for (int i = 0; i < 100; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 70000);
        auto pred = [](T) { return true; };
        
        auto simd_result = raze::algorithm::count_if(vec.begin(), vec.end(), pred);
        auto std_result = std::ranges::count_if(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 100);
    }
}

template <typename T>
void test_count_if_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 80000);
        T threshold = RandomGenerator<T>(seed + i + 80000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        
        auto simd_result = raze::algorithm::count_if(vec, pred);
        auto std_result = std::ranges::count_if(vec, pred);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_count_if_not_random(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 90000);
        T threshold = RandomGenerator<T>(seed + i + 90000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        auto not_pred = [threshold](T x) { return !(x > threshold); };
        
        auto simd_result = raze::algorithm::count_if_not(vec.begin(), vec.end(), pred);
        auto std_result = std::ranges::count_if(vec.begin(), vec.end(), not_pred);
        
        raze_assert(simd_result == std_result);
    }
    
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 100000);
        T target = RandomGenerator<T>(seed + i + 100000)();
        
        auto pred = [target](T x) { return x == target; };
        auto not_pred = [target](T x) { return x != target; };
        
        auto simd_result = raze::algorithm::count_if_not(vec.begin(), vec.end(), pred);
        auto std_result = std::ranges::count_if(vec.begin(), vec.end(), not_pred);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_count_if_not_ranges(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        auto vec = generate_random_vector<T>(100, seed + i + 110000);
        T threshold = RandomGenerator<T>(seed + i + 110000)();
        
        auto pred = [threshold](T x) { return x > threshold; };
        auto not_pred = [threshold](T x) { return !(x > threshold); };
        
        auto simd_result = raze::algorithm::count_if_not(vec, pred);
        auto std_result = std::ranges::count_if(vec, not_pred);
        
        raze_assert(simd_result == std_result);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_count_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 120000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        
        int target_x = gen();
        
        auto simd_result = raze::algorithm::count_if(vec.begin(), vec.end(), 
            [target_x](int x) { return x == target_x; },
            [](const Point& p) { return p.x; });
        
        auto std_result = std::ranges::count_if(vec.begin(), vec.end(),
            [target_x](const Point& p) { return p.x == target_x; });
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_count_edge_cases() {
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(42));
        auto std_result = std::ranges::count(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == 0);
        raze_assert(std_result == 0);
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(42));
        auto std_result = std::ranges::count(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 1);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(99));
        auto std_result = std::ranges::count(vec.begin(), vec.end(), T(99));
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 0);
    }
    
    {
        std::vector<T> vec(100, T(0));
        vec[0] = T(42);
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(42));
        auto std_result = std::ranges::count(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 1);
    }
    
    {
        std::vector<T> vec(100, T(0));
        vec[99] = T(42);
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(42));
        auto std_result = std::ranges::count(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 1);
    }
    
    {
        std::vector<T> vec(100, T(0));
        vec[50] = T(42);
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(42));
        auto std_result = std::ranges::count(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 1);
    }
    
    {
        std::vector<T> vec(100, T(42));
        auto simd_result = raze::algorithm::count(vec.begin(), vec.end(), T(42));
        auto std_result = std::ranges::count(vec.begin(), vec.end(), T(42));
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == 100);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_count_random<T>();
    test_count_ranges<T>();
    test_count_if_random<T>();
    test_count_if_ranges<T>();
    test_count_if_not_random<T>();
    test_count_if_not_ranges<T>();
    test_count_edge_cases<T>();
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
    
    test_count_with_projection<int>();
    
    return 0;
}