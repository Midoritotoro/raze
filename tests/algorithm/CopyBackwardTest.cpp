#include <raze/algorithm/copy/CopyBackward.h>

#include <vector>
#include <list>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cstring>

template <typename T>
struct IntDistributionType { using type = T; };
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

template <typename Range1, typename Range2>
bool equal_ranges(const Range1& lhs, const Range2& rhs) {
    return std::ranges::equal(
        lhs, rhs,
        [](const auto& a, const auto& b) {
            using T = std::remove_cvref_t<decltype(a)>;

            if constexpr (std::is_floating_point_v<T>) return (std::isnan(a) && std::isnan(b)) || (a == b);
            else return a == b;
        });
}

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
    for (auto& elem : vec) elem = gen();
    return vec;
}

template <typename T>
std::list<T> generate_random_list(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::list<T> lst;
    for (size_t i = 0; i < size; ++i) lst.push_back(gen());
    return lst;
}

struct S1 { char a; friend bool operator==(const S1&, const S1&) = default; };
struct S2 { short a; friend bool operator==(const S2&, const S2&) = default; };
struct S4 { int a; friend bool operator==(const S4&, const S4&) = default; };
struct S8 { int x, y; friend bool operator==(const S8&, const S8&) = default; };
struct Point { int x, y; friend bool operator==(const Point&, const Point&) = default; };

template <typename T>
void test_copy_backward_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto src = generate_random_vector<T>(size, seed + i);
        std::vector<T> dst(size), dst_copy(size);
        
        auto simd_result = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto std_result = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - simd_result.in) == (src.end() - std_result.in));
        raze_assert((dst.end() - simd_result.out) == (dst_copy.end() - std_result.out));
    }
}

template <typename T>
void test_copy_backward_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto src = generate_random_vector<T>(size, seed + i + 30000);
        std::vector<T> dst(size), dst_copy(size);
        
        auto simd_result = raze::algorithm::copy_backward(src, dst.end());
        auto std_result = std::ranges::copy_backward(src, dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - simd_result.in) == (src.end() - std_result.in));
        raze_assert((dst.end() - simd_result.out) == (dst_copy.end() - std_result.out));
    }
}

template <typename T>
void test_copy_backward_edge_cases() {
    {
        std::vector<T> src, dst, dst_copy;
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    {
        std::vector<T> src = {T(42)};
        std::vector<T> dst(1), dst_copy(1);
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    {
        std::vector<T> src(100, T(99));
        std::vector<T> dst(100), dst_copy(100);
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        raze_assert(equal_ranges(dst, dst_copy));
        for (const auto& x : dst) raze_assert(x == T(99));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
}

template <typename T>
void test_copy_backward_return_value(unsigned seed = 42) {
    {
        std::vector<T> src = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> dst(5), dst_copy(5);
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    
    {
        std::vector<T> src = {T(10), T(20), T(30)};
        std::vector<T> dst(10, T(0)), dst_copy(10, T(0));
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    
    {
        std::vector<T> src = {T(1), T(2), T(3), T(4), T(5), T(6), T(7), T(8)};
        std::vector<T> dst(8, T(0)), dst_copy(8, T(0));
        
        auto r1 = raze::algorithm::copy_backward(src.begin() + 2, src.begin() + 6, dst.end());
        auto r2 = std::ranges::copy_backward(src.begin() + 2, src.begin() + 6, dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<T> src(size);
        std::vector<T> dst(size), dst_copy(size);
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    
    {
        std::vector<T> src = {T(10), T(20), T(30), T(40)};
        std::vector<T> dst(4), dst_copy(4);
        
        auto r1 = raze::algorithm::copy_backward(src, dst.end());
        auto r2 = std::ranges::copy_backward(src, dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
}

template <typename T>
void test_copy_backward_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        auto src = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        std::vector<T> dst(size), dst_copy(size);
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
}

template <typename T>
void test_copy_backward_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto src = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        std::vector<T> dst(size), dst_copy(size);
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
}

template <typename T>
void test_copy_backward_list(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 200);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        auto src = generate_random_list<T>(size, seed + i + 200000);
        std::list<T> dst, dst_copy;
        
        for (size_t j = 0; j < size; ++j) { dst.push_back(T{}); dst_copy.push_back(T{}); }
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        raze_assert(equal_ranges(dst, dst_copy));
    }
}

template <typename T>
void test_copy_backward_overlapping(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        size_t size = 100;
        auto vec = generate_random_vector<T>(size, seed + i + 1600000);
        auto vec_copy = vec;
        
        auto r1 = raze::algorithm::copy_backward(vec.begin(), vec.begin() + 50, vec.begin() + 100);
        auto r2 = std::ranges::copy_backward(vec_copy.begin(), vec_copy.begin() + 50, vec_copy.begin() + 100);
        
        raze_assert(equal_ranges(vec, vec_copy));
        
        raze_assert((vec.end() - r1.in) == (vec_copy.end() - r2.in));
        raze_assert((vec.end() - r1.out) == (vec_copy.end() - r2.out));
        
        for (size_t j = 0; j < 50; ++j) {
            if constexpr (std::is_floating_point_v<T>) { if (std::isnan(vec[j]) || std::isnan(vec_copy[j])) continue; }
            raze_assert(vec[j] == vec_copy[j]);
        }
        for (size_t j = 0; j < 50; ++j) {
            if constexpr (std::is_floating_point_v<T>) { if (std::isnan(vec[50 + j]) || std::isnan(vec_copy[j])) continue; }
            raze_assert(vec[50 + j] == vec_copy[j]);
        }
    }
    
    for (int i = 0; i < 50; ++i) {
        size_t size = 100;
        auto vec = generate_random_vector<T>(size, seed + i + 1700000);
        auto vec_copy = vec;
        
        auto r1 = raze::algorithm::copy_backward(vec.begin(), vec.end(), vec.end());
        auto r2 = std::ranges::copy_backward(vec_copy.begin(), vec_copy.end(), vec_copy.end());
        
        raze_assert(equal_ranges(vec, vec_copy));
        raze_assert((vec.end() - r1.in) == (vec_copy.end() - r2.in));
        raze_assert((vec.end() - r1.out) == (vec_copy.end() - r2.out));
    }
}

template <class Struct>
void test_copy_backward_small_structs(unsigned seed = 42) {
    std::mt19937 size_gen(seed);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        std::vector<Struct> src(size), dst(size), dst_copy(size);
        for (auto& x : src) std::memset(&x, static_cast<unsigned char>(i % 256), sizeof(Struct));
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    
    for (size_t size : {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<Struct> src(size), dst(size), dst_copy(size);
        for (auto& x : src) std::memset(&x, 0xAB, sizeof(Struct));
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
    
    for (int i = 0; i < 50; ++i) {
        std::vector<Struct> vec(100);
        auto vec_copy = vec;
        for (auto& x : vec) std::memset(&x, 0xCD, sizeof(Struct));
        vec_copy = vec;
        
        auto r1 = raze::algorithm::copy_backward(vec.begin(), vec.begin() + 50, vec.end());
        auto r2 = std::ranges::copy_backward(vec_copy.begin(), vec_copy.begin() + 50, vec_copy.end());
        
        raze_assert(equal_ranges(vec, vec_copy));
        raze_assert((vec.end() - r1.in) == (vec_copy.end() - r2.in));
        raze_assert((vec.end() - r1.out) == (vec_copy.end() - r2.out));
    }
}

template <class T>
void test_copy_backward_bit_patterns() {
    unsigned char patterns[] = {0x00, 0xFF, 0xAA, 0x55, 0x01, 0x80};
    for (size_t size : {0, 1, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 1000}) {
        for (unsigned char pattern : patterns) {
            std::vector<T> src(size), dst(size), dst_copy(size);
            for (auto& x : src) std::memset(&x, pattern, sizeof(T));
            
            auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
            auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
            
            raze_assert(equal_ranges(dst, dst_copy));
            raze_assert((src.end() - r1.in) == (src.end() - r2.in));
            raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
            
            for (size_t i = 0; i < size; ++i) {
                raze_assert(std::memcmp(&dst[i], &src[i], sizeof(T)) == 0);
            }
        }
    }
}

void test_copy_backward_struct(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> src(100), dst(100), dst_copy(100);
        for (auto& p : src) { p.x = i; p.y = i * 2; }
        
        auto r1 = raze::algorithm::copy_backward(src.begin(), src.end(), dst.end());
        auto r2 = std::ranges::copy_backward(src.begin(), src.end(), dst_copy.end());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert((src.end() - r1.in) == (src.end() - r2.in));
        raze_assert((dst.end() - r1.out) == (dst_copy.end() - r2.out));
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_copy_backward_random<T>();
    test_copy_backward_ranges<T>();
    test_copy_backward_edge_cases<T>();
    test_copy_backward_return_value<T>();
    test_copy_backward_simd_boundaries<T>();
    test_copy_backward_large_vectors<T>();
    test_copy_backward_list<T>();
    test_copy_backward_overlapping<T>();
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
    
    test_copy_backward_struct();
    test_copy_backward_small_structs<S1>();
    test_copy_backward_small_structs<S2>();
    test_copy_backward_small_structs<S4>();
    test_copy_backward_small_structs<S8>();
    
    test_copy_backward_bit_patterns<S1>();
    test_copy_backward_bit_patterns<S2>();
    test_copy_backward_bit_patterns<S4>();
    test_copy_backward_bit_patterns<S8>();
    test_copy_backward_bit_patterns<float>();
    test_copy_backward_bit_patterns<double>();
    
    return 0;
}