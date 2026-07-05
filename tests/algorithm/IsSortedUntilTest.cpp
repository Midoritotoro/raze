#include <raze/algorithm/order/IsSortedUntil.h>

#include <vector>
#include <array>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>

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
std::vector<T> generate_sorted_vector(size_t size, unsigned seed = 42) {
    auto vec = generate_random_vector<T>(size, seed);
    std::sort(vec.begin(), vec.end());
    return vec;
}

template <typename T>
std::vector<T> generate_partially_sorted_vector(size_t size, size_t break_pos, unsigned seed = 42) {
    auto vec = generate_sorted_vector<T>(size, seed);
    if (break_pos < size && break_pos + 1 < size) {
        std::swap(vec[break_pos], vec[break_pos + 1]);
    }
    return vec;
}

template <typename T, std::size_t N>
void test_is_sorted_until_array_sorted() {
    std::array<T, N> arr{};
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = static_cast<T>(i);
    }
    
    auto simd_result = raze::algorithm::is_sorted_until(arr);
    auto std_result = std::ranges::is_sorted_until(arr);
    
    raze_assert(simd_result == std_result);
}

template <typename T, std::size_t N>
void test_is_sorted_until_array_unsorted() {
    std::array<T, N> arr{};
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = static_cast<T>(i);
    }
    
    if constexpr (N > 1) {
        std::swap(arr[N / 2], arr[N / 2 + 1]);
        
        auto simd_result = raze::algorithm::is_sorted_until(arr);
        auto std_result = std::ranges::is_sorted_until(arr);
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result != arr.end());
        if (simd_result != arr.begin()) {
            auto prev = simd_result;
            --prev;
            raze_assert(*prev > *simd_result);
        }
    }
}

template <typename T, std::size_t N>
void test_is_sorted_until_array_with_pred() {
    std::array<T, N> arr{};
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = static_cast<T>(N - i);
    }
    
    auto pred = [](T a, T b) { return a >= b; };
    
    auto simd_result = raze::algorithm::is_sorted_until(arr, pred);
    auto std_result = std::ranges::is_sorted_until(arr, pred);
    
    raze_assert(simd_result == std_result);
    raze_assert(simd_result == arr.end());
}

template <typename T, std::size_t N>
void test_is_sorted_until_array_with_proj() {
    std::array<T, N> arr{};
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = static_cast<T>(i);
    }
    
    auto proj = [](T x) { return -x; };
    
    auto simd_result = raze::algorithm::is_sorted_until(arr, std::less<T>{}, proj);
    auto std_result = std::ranges::is_sorted_until(arr, std::less<T>{}, proj);
    
    raze_assert(simd_result == std_result);
    raze_assert(simd_result != arr.end());
}

template <typename T>
void test_is_sorted_until_arrays_compile_time_sizes() {
    test_is_sorted_until_array_sorted<T, 0>();
    test_is_sorted_until_array_sorted<T, 1>();
    test_is_sorted_until_array_sorted<T, 4>();
    test_is_sorted_until_array_sorted<T, 8>();
    test_is_sorted_until_array_sorted<T, 16>();
    test_is_sorted_until_array_sorted<T, 32>();
    test_is_sorted_until_array_sorted<T, 64>();
    test_is_sorted_until_array_sorted<T, 128>();
    test_is_sorted_until_array_sorted<T, 256>();
    
    test_is_sorted_until_array_unsorted<T, 4>();
    test_is_sorted_until_array_unsorted<T, 8>();
    test_is_sorted_until_array_unsorted<T, 16>();
    test_is_sorted_until_array_unsorted<T, 32>();
    test_is_sorted_until_array_unsorted<T, 64>();
    test_is_sorted_until_array_unsorted<T, 128>();
    test_is_sorted_until_array_unsorted<T, 256>();
    
    test_is_sorted_until_array_with_pred<T, 16>();
    test_is_sorted_until_array_with_pred<T, 64>();
    
    test_is_sorted_until_array_with_proj<T, 16>();
    test_is_sorted_until_array_with_proj<T, 64>();
}

template <typename T>
void test_is_sorted_until_random(unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);

        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        
        raze_assert(simd_result == std_result);
        if (simd_result != vec.end() && simd_result != vec.begin()) {
            auto prev = simd_result;
            --prev;
            raze_assert(*prev > *simd_result);
        }
    }
}

template <typename T>
void test_is_sorted_until_sorted(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_sorted_vector<T>(size, seed + i);
        
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == vec.end());
    }
}

template <typename T>
void test_is_sorted_until_partially_sorted(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 1000);
    std::uniform_int_distribution<int> pos_dist(0, 998);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        size_t break_pos = std::min(static_cast<size_t>(pos_dist(size_gen)), size - 2);
        auto vec = generate_partially_sorted_vector<T>(size, break_pos, seed + i);
        
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        
        raze_assert(simd_result == std_result);
        if (simd_result != vec.end() && simd_result != vec.begin()) {
            auto prev = simd_result;
            --prev;
            raze_assert(*prev > *simd_result);
        }
    }
}

template <typename T>
void test_is_sorted_until_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        
        auto simd_result = raze::algorithm::is_sorted_until(vec);
        auto std_result = std::ranges::is_sorted_until(vec);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_is_sorted_until_with_predicate(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 40000);
        
        auto pred = [](T a, T b) { return a > b; };
        
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end(), pred);
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end(), pred);
        
        raze_assert(simd_result == std_result);
        if (simd_result != vec.end() && simd_result != vec.begin()) {
            auto prev = simd_result;
            --prev;
            raze_assert(!pred(*prev, *simd_result));
        }
    }
}

template <typename T>
void test_is_sorted_until_with_projection(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 50000);
        
        auto proj = [](T x) { return -x; };
        
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end(), std::less<T>{}, proj);
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end(), std::less<T>{}, proj);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_is_sorted_until_edge_cases() {
    {
        std::vector<T> vec;
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        raze_assert(simd_result == vec.end());
        raze_assert(std_result == vec.end());
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec = {T(42)};
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        raze_assert(simd_result == vec.end());
        raze_assert(std_result == vec.end());
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec = {T(1), T(2), T(3), T(4), T(5)};
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        raze_assert(simd_result == vec.end());
        raze_assert(std_result == vec.end());
        raze_assert(simd_result == std_result);
    }
    
    {
        std::vector<T> vec = {T(5), T(4), T(3), T(2), T(1)};
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        raze_assert(simd_result == std_result);
        if (simd_result != vec.begin()) {
            raze_assert(simd_result == vec.begin() + 1);
        }
    }
    
    {
        std::vector<T> vec = {T(1), T(3), T(2), T(4), T(5)};
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        raze_assert(simd_result == std_result);
        if (simd_result != vec.begin()) {
            raze_assert(simd_result == vec.begin() + 2);
        }
    }
    
    {
        std::vector<T> vec(100, T(42));
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        raze_assert(simd_result == vec.end());
        raze_assert(std_result == vec.end());
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void test_is_sorted_until_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        auto vec = generate_sorted_vector<T>(size, seed + static_cast<unsigned>(size));
        
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
        
        raze_assert(simd_result == std_result);
        raze_assert(simd_result == vec.end());
    }
    
    for (size_t size : {2, 4, 8, 16, 32, 64, 128, 256, 512}) {
        if (size >= 2) {
            auto vec = generate_partially_sorted_vector<T>(size, size / 2, seed + static_cast<unsigned>(size) + 1000);
            
            auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end());
            auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end());
            
            raze_assert(simd_result == std_result);
        }
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_is_sorted_until_with_struct_projection(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 120000);
        for (auto& p : vec) {
            p.x = gen();
            p.y = gen();
        }
        
        auto proj = [](const Point& p) { return p.x; };
        
        auto simd_result = raze::algorithm::is_sorted_until(vec.begin(), vec.end(), std::less<int>{}, proj);
        auto std_result = std::ranges::is_sorted_until(vec.begin(), vec.end(), std::less<int>{}, proj);
        
        raze_assert(simd_result == std_result);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_is_sorted_until_random<T>();
    test_is_sorted_until_sorted<T>();
    test_is_sorted_until_partially_sorted<T>();
    test_is_sorted_until_ranges<T>();
    test_is_sorted_until_with_predicate<T>();
    test_is_sorted_until_with_projection<T>();
    test_is_sorted_until_edge_cases<T>();
    test_is_sorted_until_simd_boundaries<T>();
    test_is_sorted_until_arrays_compile_time_sizes<T>();
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
    
    test_is_sorted_until_with_struct_projection<int>();
    
    return 0;
}