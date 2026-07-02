#include <raze/algorithm/copy/CopyN.h>

#include <vector>
#include <list>
#include <forward_list>
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

struct S1 { char a; friend bool operator==(const S1&, const S1&) = default; };
struct S2 { short a; friend bool operator==(const S2&, const S2&) = default; };
struct S4 { int a; friend bool operator==(const S4&, const S4&) = default; };
struct S8 { int x, y; friend bool operator==(const S8&, const S8&) = default; };
struct Point { int x, y; friend bool operator==(const Point&, const Point&) = default; };

template <typename T>
void test_copy_n_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto src = generate_random_vector<T>(size, seed + i + 60000);
        std::vector<T> dst(size), dst_copy(size);
        
        std::uniform_int_distribution<size_t> n_dist(0, size);
        size_t n = n_dist(size_gen);
        
        auto simd_result = raze::algorithm::copy_n(src.begin(), n, dst.begin());
        auto std_result = std::copy_n(src.begin(), n, dst_copy.begin());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert(simd_result.in == src.begin() + n);
        raze_assert(simd_result.out == dst.begin() + n);
        raze_assert(std_result == dst_copy.begin() + n);
    }
}

template <typename T>
void test_copy_n_edge_cases() {
    {
        std::vector<T> src = {T(1), T(2), T(3)};
        std::vector<T> dst(3), dst_copy(3);
        auto r1 = raze::algorithm::copy_n(src.begin(), 0, dst.begin());
        auto r2 = std::copy_n(src.begin(), 0, dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert(r1.in == src.begin() && r1.out == dst.begin());
        raze_assert(r2 == dst_copy.begin());
    }
    {
        std::vector<T> src = {T(1), T(2), T(3)};
        std::vector<T> dst(3), dst_copy(3);
        auto r1 = raze::algorithm::copy_n(src.begin(), 3, dst.begin());
        auto r2 = std::copy_n(src.begin(), 3, dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert(r1.in == src.end() && r1.out == dst.end());
        raze_assert(r2 == dst_copy.end());
    }
    {
        std::vector<T> src(100, T(42));
        std::vector<T> dst(50), dst_copy(50);
        raze::algorithm::copy_n(src.begin(), 50, dst.begin());
        std::copy_n(src.begin(), 50, dst_copy.begin());
        raze_assert(equal_ranges(dst, dst_copy));
        for (const auto& x : dst) raze_assert(x == T(42));
    }
}

template <typename T>
void test_copy_n_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        auto src = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        std::vector<T> dst(size), dst_copy(size);
        
        std::uniform_int_distribution<size_t> n_dist(0, size);
        std::mt19937 gen(seed + static_cast<unsigned>(size));
        size_t n = n_dist(gen);
        
        auto r1 = raze::algorithm::copy_n(src.begin(), n, dst.begin());
        auto r2 = std::copy_n(src.begin(), n, dst_copy.begin());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert(r1.in == src.begin() + n);
        raze_assert(r1.out == dst.begin() + n);
        raze_assert(r2 == dst_copy.begin() + n);
    }
}

template <typename T>
void test_copy_n_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto src = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        std::vector<T> dst(size), dst_copy(size);
        
        raze::algorithm::copy_n(src.begin(), size, dst.begin());
        std::copy_n(src.begin(), size, dst_copy.begin());
        
        raze_assert(equal_ranges(dst, dst_copy));
    }
}

template <class Struct>
void test_copy_n_small_structs(unsigned seed = 42) {
    std::mt19937 size_gen(seed);
    std::uniform_int_distribution<int> size_dist(10, 500);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        std::vector<Struct> src(size), dst(size), dst_copy(size);
        for (auto& x : src) std::memset(&x, static_cast<unsigned char>(i % 256), sizeof(Struct));
        
        std::uniform_int_distribution<size_t> n_dist(0, size);
        size_t n = n_dist(size_gen);
        
        auto r1 = raze::algorithm::copy_n(src.begin(), n, dst.begin());
        auto r2 = std::copy_n(src.begin(), n, dst_copy.begin());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert(r1.in == src.begin() + n);
        raze_assert(r1.out == dst.begin() + n);
        raze_assert(r2 == dst_copy.begin() + n);
    }
}

template <class T>
void test_copy_n_bit_patterns() {
    unsigned char patterns[] = {0x00, 0xFF, 0xAA, 0x55, 0x01, 0x80};
    for (size_t size : {1, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 1000}) {
        for (unsigned char pattern : patterns) {
            std::vector<T> src(size), dst(size), dst_copy(size);
            for (auto& x : src) std::memset(&x, pattern, sizeof(T));
            
            raze::algorithm::copy_n(src.begin(), size, dst.begin());
            std::copy_n(src.begin(), size, dst_copy.begin());
            
            raze_assert(equal_ranges(dst, dst_copy));
            for (size_t i = 0; i < size; ++i) {
                raze_assert(std::memcmp(&dst[i], &src[i], sizeof(T)) == 0);
            }
        }
    }
}

void test_copy_n_struct(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> src(100), dst(100), dst_copy(100);
        for (auto& p : src) { p.x = i; p.y = i * 2; }
        
        auto r1 = raze::algorithm::copy_n(src.begin(), 50, dst.begin());
        auto r2 = std::copy_n(src.begin(), 50, dst_copy.begin());
        
        raze_assert(equal_ranges(dst, dst_copy));
        raze_assert(r1.in == src.begin() + 50);
        raze_assert(r1.out == dst.begin() + 50);
        raze_assert(r2 == dst_copy.begin() + 50);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_copy_n_random<T>();
    test_copy_n_edge_cases<T>();
    test_copy_n_simd_boundaries<T>();
    test_copy_n_large_vectors<T>();
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
    
    test_copy_n_struct();
    test_copy_n_small_structs<S1>();
    test_copy_n_small_structs<S2>();
    test_copy_n_small_structs<S4>();
    test_copy_n_small_structs<S8>();
    
    test_copy_n_bit_patterns<S1>();
    test_copy_n_bit_patterns<S2>();
    test_copy_n_bit_patterns<S4>();
    test_copy_n_bit_patterns<S8>();
    test_copy_n_bit_patterns<float>();
    test_copy_n_bit_patterns<double>();
    
    return 0;
}