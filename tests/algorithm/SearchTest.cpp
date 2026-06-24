#include <raze/algorithm/find/Search.h>

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
void test_search_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 1000);
    std::uniform_int_distribution<int> sub_size_dist(1, 50);
    
    for (int i = 0; i < 1000; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 100000);
        
        auto simd_result = raze::algorithm::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size / 2);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i + 200000);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 300000);
        
        std::uniform_int_distribution<size_t> pos_dist(0, main_size - sub_size);
        size_t pos = pos_dist(size_gen);
        
        for (size_t j = 0; j < sub_size; ++j) {
            vec1[pos + j] = vec2[j];
        }
        
        auto simd_result = raze::algorithm::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
}

template <typename T>
void test_search_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 1000);
    std::uniform_int_distribution<int> sub_size_dist(1, 50);
    
    for (int i = 0; i < 1000; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i + 400000);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 500000);
        
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
    
    for (int i = 0; i < 1000; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size / 2);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i + 600000);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 700000);
        
        std::uniform_int_distribution<size_t> pos_dist(0, main_size - sub_size);
        size_t pos = pos_dist(size_gen);
        
        for (size_t j = 0; j < sub_size; ++j) {
            vec1[pos + j] = vec2[j];
        }
        
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
}

template <typename T>
void test_search_pred_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 1000);
    std::uniform_int_distribution<int> sub_size_dist(1, 50);
    
    for (int i = 0; i < 1000; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i + 800000);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 900000);
        
        auto pred = [](const T& a, const T& b) { return a == b; };
        
        auto simd_result = raze::algorithm::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred);
        auto std_result = std::ranges::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size / 2);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i + 1000000);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 1100000);
        
        auto pred_true = [](const T&, const T&) { return true; };
        
        auto simd_result = raze::algorithm::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_true);
        auto std_result = std::ranges::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_true);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.begin());
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size / 2);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i + 1200000);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 1300000);
        
        auto pred_false = [](const T&, const T&) { return false; };
        
        auto simd_result = raze::algorithm::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_false);
        auto std_result = std::ranges::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred_false);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.end());
    }
}

template <typename T>
void test_search_pred_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 1000);
    std::uniform_int_distribution<int> sub_size_dist(1, 50);
    
    for (int i = 0; i < 1000; ++i) {
        size_t main_size = size_dist(size_gen);
        size_t sub_size = std::min(static_cast<size_t>(sub_size_dist(size_gen)), main_size);
        
        auto vec1 = generate_random_vector<T>(main_size, seed + i + 1400000);
        auto vec2 = generate_random_vector<T>(sub_size, seed + i + 1500000);
        
        auto pred = [](const T& a, const T& b) { return a == b; };
        
        auto simd_result = raze::algorithm::search(vec1, vec2, pred);
        auto std_result = std::ranges::search(vec1, vec2, pred);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_search_with_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec1(100);
        std::vector<Point> vec2(10);
        RandomGenerator<int> gen(seed + i + 1600000);
        for (size_t j = 0; j < 100; ++j) {
            vec1[j].x = gen();
            vec1[j].y = gen();
        }
        for (size_t j = 0; j < 10; ++j) {
            vec2[j].x = gen();
            vec2[j].y = gen();
        }
        
        size_t pos = 45;
        for (size_t j = 0; j < 10; ++j) {
            vec1[pos + j].x = vec2[j].x;
        }
        
        auto proj = [](const Point& p) { return p.x; };
        auto pred = [](int a, int b) { return a == b; };
        
        auto simd_result = raze::algorithm::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred, proj, proj);
        auto std_result = std::ranges::search(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), pred, proj, proj);
        
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        
        auto simd_result2 = raze::algorithm::search(vec1, vec2, pred, proj, proj);
        auto std_result2 = std::ranges::search(vec1, vec2, pred, proj, proj);
        
        raze_assert(simd_result2.begin() == std_result2.begin());
        raze_assert(simd_result2.end() == std_result2.end());
    }
}

template <typename T>
void test_search_edge_cases() {
    {
        std::vector<T> vec1 = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> vec2 = {T(3), T(4)};
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() - vec1.begin() == 2);
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(4), T(5)};
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2;
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.begin());
    }
    
    {
        std::vector<T> vec1;
        std::vector<T> vec2 = {T(1), T(2)};
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(1), T(2), T(3)};
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.begin());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(1), T(2), T(3), T(4)};
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.end());
    }
    
    {
        std::vector<T> vec1(1000, T(42));
        std::vector<T> vec2(10, T(42));
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() == vec1.begin());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(1), T(2)};
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() - vec1.begin() == 0);
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3), T(1), T(2)};
        std::vector<T> vec2 = {T(1), T(2)};
        auto simd_result = raze::algorithm::search(vec1, vec2);
        auto std_result = std::ranges::search(vec1, vec2);
        raze_assert(simd_result.begin() == std_result.begin());
        raze_assert(simd_result.end() == std_result.end());
        raze_assert(simd_result.begin() - vec1.begin() == 0);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_search_random<T>();
    test_search_ranges<T>();
    test_search_pred_random<T>();
    test_search_pred_ranges<T>();
    test_search_edge_cases<T>();
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
    
    test_search_with_projection<int>();
    
    return 0;
}