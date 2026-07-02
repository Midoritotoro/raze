#include <raze/algorithm/fill/Fill.h>
#include <raze/algorithm/fill/FillN.h>

#include <vector>
#include <list>
#include <forward_list>
#include <random>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cstring>

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

template <typename T>
std::forward_list<T> generate_random_forward_list(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::forward_list<T> flst;
    std::vector<T> temp;
    for (size_t i = 0; i < size; ++i) temp.push_back(gen());
    for (auto it = temp.rbegin(); it != temp.rend(); ++it) flst.push_front(*it);
    return flst;
}

struct S1 { char a; friend bool operator==(const S1&, const S1&) = default; };
struct S2 { short a; friend bool operator==(const S2&, const S2&) = default; };
struct S4 { int a; friend bool operator==(const S4&, const S4&) = default; };

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};
static_assert(sizeof(Point) == 8);
static_assert(std::is_trivially_copyable_v<Point>);

template <typename T>
void test_fill_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    RandomGenerator<T> gen(seed);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        auto vec_copy = vec;
        T val = gen();
        
        raze::algorithm::fill(vec.begin(), vec.end(), val);
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), val);
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_fill_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    RandomGenerator<T> gen(seed);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        auto vec_copy = vec;
        T val = gen();
        
        raze::algorithm::fill(vec, val);
        std::ranges::fill(vec_copy, val);
        
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_fill_n_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    RandomGenerator<T> gen(seed);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 60000);
        auto vec_copy = vec;
        
        std::uniform_int_distribution<size_t> n_dist(0, size);
        size_t n = n_dist(size_gen);
        T val = gen();
        
        auto simd_result = raze::algorithm::fill_n(vec.begin(), n, val);
        auto std_result = std::fill_n(vec_copy.begin(), n, val);
        
        raze_assert(vec == vec_copy);
        raze_assert(simd_result == vec.begin() + n);
        raze_assert(std_result == vec_copy.begin() + n);
    }
}

template <typename T>
void test_fill_edge_cases() {
    {
        std::vector<T> vec, vec_copy;
        raze::algorithm::fill(vec.begin(), vec.end(), T(42));
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), T(42));
        raze_assert(vec == vec_copy);
    }
    {
        std::vector<T> vec = {T(1)}, vec_copy = vec;
        raze::algorithm::fill(vec.begin(), vec.end(), T(42));
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), T(42));
        raze_assert(vec == vec_copy && vec[0] == T(42));
    }
    {
        std::vector<T> vec(100, T(1)), vec_copy = vec;
        raze::algorithm::fill(vec.begin(), vec.end(), T(99));
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), T(99));
        raze_assert(vec == vec_copy);
    }
    {
        std::vector<T> vec = {T(1), T(2), T(3)}, vec_copy = vec;
        auto res = raze::algorithm::fill_n(vec.begin(), 0, T(42));
        auto res_copy = std::fill_n(vec_copy.begin(), 0, T(42));
        raze_assert(vec == vec_copy && res == vec.begin() && res_copy == vec_copy.begin());
    }
    {
        std::vector<T> vec = {T(1), T(2), T(3)}, vec_copy = vec;
        auto res = raze::algorithm::fill_n(vec.begin(), 3, T(42));
        auto res_copy = std::fill_n(vec_copy.begin(), 3, T(42));
        raze_assert(vec == vec_copy && res == vec.end() && res_copy == vec_copy.end());
    }
}

template <typename T>
void test_fill_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        std::vector<T> vec(size, T(0)), vec_copy(size, T(0));
        raze::algorithm::fill(vec.begin(), vec.end(), T(42));
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), T(42));
        raze_assert(vec == vec_copy);
    }
    
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<T> vec(size, T(0)), vec_copy(size, T(0));
        std::uniform_int_distribution<size_t> n_dist(0, size);
        std::mt19937 gen(seed + static_cast<unsigned>(size));
        size_t n = n_dist(gen);
        
        auto res = raze::algorithm::fill_n(vec.begin(), n, T(99));
        auto res_copy = std::fill_n(vec_copy.begin(), n, T(99));
        
        raze_assert(vec == vec_copy && res == vec.begin() + n && res_copy == vec_copy.begin() + n);
    }
}

template <typename T>
void test_fill_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec_copy = vec;
        raze::algorithm::fill(vec.begin(), vec.end(), T(99));
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), T(99));
        raze_assert(vec == vec_copy);
    }
}

template <typename T>
void test_fill_list(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 200);
    RandomGenerator<T> gen(seed);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        auto lst = generate_random_list<T>(size, seed + i + 200000);
        auto lst_copy = lst;
        T val = gen();
        raze::algorithm::fill(lst.begin(), lst.end(), val);
        std::ranges::fill(lst_copy.begin(), lst_copy.end(), val);
        raze_assert(lst == lst_copy);
    }
}

template <typename T>
void test_fill_forward_list(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 200);
    RandomGenerator<T> gen(seed);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        auto flst = generate_random_forward_list<T>(size, seed + i + 600000);
        auto flst_copy = flst;
        T val = gen();
        raze::algorithm::fill(flst.begin(), flst.end(), val);
        std::ranges::fill(flst_copy.begin(), flst_copy.end(), val);
        raze_assert(flst == flst_copy);
    }
}

template <class Struct>
void test_fill_small_structs(unsigned seed = 42) {
    std::mt19937 size_gen(seed);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        std::vector<Struct> vec(size), vec_copy(size);
        
        Struct val{};
        std::memset(&val, static_cast<unsigned char>(i % 256), sizeof(Struct));
        
        raze::algorithm::fill(vec.begin(), vec.end(), val);
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), val);
        raze_assert(vec == vec_copy);
    }
    
    for (size_t size : {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<Struct> vec(size), vec_copy(size);
        Struct val{};
        std::memset(&val, 0xAB, sizeof(Struct));
        
        raze::algorithm::fill(vec.begin(), vec.end(), val);
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), val);
        raze_assert(vec == vec_copy);
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size = 100;
        std::vector<Struct> vec(size), vec_copy(size);
        Struct val{};
        std::memset(&val, 0x55, sizeof(Struct));
        
        size_t n = 50 + (i % 50);
        auto res = raze::algorithm::fill_n(vec.begin(), n, val);
        auto res_copy = std::fill_n(vec_copy.begin(), n, val);
        
        raze_assert(vec == vec_copy);
        raze_assert(res == vec.begin() + n);
        raze_assert(res_copy == vec_copy.begin() + n);
    }
}

template <class T>
void test_fill_bit_patterns() {
    unsigned char patterns[] = {0x00, 0xFF, 0xAA, 0x55, 0x01, 0x80};
    for (size_t size : {0, 1, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 1000}) {
        for (unsigned char pattern : patterns) {
            std::vector<T> vec(size), vec_copy(size);
            T val{};
            std::memset(&val, pattern, sizeof(T));
            
            raze::algorithm::fill(vec.begin(), vec.end(), val);
            std::ranges::fill(vec_copy.begin(), vec_copy.end(), val);
            
            for (size_t i = 0; i < size; ++i) {
                raze_assert(std::memcmp(&vec[i], &val, sizeof(T)) == 0);
            }
            raze_assert(vec.size() == vec_copy.size());
        }
    }
}

void test_fill_struct(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100), vec_copy(100);
        Point p{42, 99};
        raze::algorithm::fill(vec.begin(), vec.end(), p);
        std::ranges::fill(vec_copy.begin(), vec_copy.end(), p);
        raze_assert(vec == vec_copy);
    }
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec(100), vec_copy(100);
        Point p{77, 88};
        auto res = raze::algorithm::fill_n(vec.begin(), 50, p);
        auto res_copy = std::fill_n(vec_copy.begin(), 50, p);
        raze_assert(vec == vec_copy && res == vec.begin() + 50 && res_copy == vec_copy.begin() + 50);
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_fill_random<T>();
    test_fill_ranges<T>();
    test_fill_n_random<T>();
    test_fill_edge_cases<T>();
    test_fill_simd_boundaries<T>();
    test_fill_large_vectors<T>();
    test_fill_list<T>();
    test_fill_forward_list<T>();
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
    
    test_fill_struct();
    
    test_fill_small_structs<S1>();
    test_fill_small_structs<S2>();
    test_fill_small_structs<S4>();
    test_fill_small_structs<Point>();
    
    test_fill_bit_patterns<S1>();
    test_fill_bit_patterns<S2>();
    test_fill_bit_patterns<S4>();
    test_fill_bit_patterns<Point>();
    test_fill_bit_patterns<float>();
    test_fill_bit_patterns<double>();
    test_fill_bit_patterns<long long>();
    
    return 0;
}