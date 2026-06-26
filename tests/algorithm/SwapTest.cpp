#include <raze/algorithm/swap/Swap.h>

#include <vector>
#include <list>
#include <forward_list>
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
std::list<T> generate_random_list(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::list<T> lst;
    for (size_t i = 0; i < size; ++i) {
        lst.push_back(gen());
    }
    return lst;
}

template <typename T>
std::forward_list<T> generate_random_forward_list(size_t size, unsigned seed = 42) {
    RandomGenerator<T> gen(seed);
    std::forward_list<T> flst;
    std::vector<T> temp;
    for (size_t i = 0; i < size; ++i) {
        temp.push_back(gen());
    }
    for (auto it = temp.rbegin(); it != temp.rend(); ++it) {
        flst.push_front(*it);
    }
    return flst;
}

template <typename T>
void test_swap_ranges_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    
    for (int i = 0; i < 1000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i);
        auto vec2 = generate_random_vector<T>(size, seed + i + 100000);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::swap_ranges(vec1_copy.begin(), vec1_copy.end(), vec2_copy.begin(), vec2_copy.end());
        
        raze_assert(std::ranges::equal(vec1, vec1_copy));
        raze_assert(std::ranges::equal(vec2, vec2_copy));
        raze_assert(simd_result.in1 == vec1.end());
        raze_assert(simd_result.in2 == vec2.end());
    }
    
    for (int i = 0; i < 500; ++i) {
        size_t size = size_dist(size_gen);
        auto vec1 = generate_random_vector<T>(size, seed + i + 200000);
        auto vec2 = generate_random_vector<T>(size, seed + i + 300000);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        std::ranges::swap_ranges(vec1_copy.begin(), vec1_copy.end(), vec2_copy.begin(), vec2_copy.end());
        
        raze_assert(std::ranges::equal(vec1, vec1_copy));
        raze_assert(std::ranges::equal(vec2, vec2_copy));
    }
}

template <typename T>
void test_swap_ranges_different_sizes(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(1, 500);
    
    for (int i = 0; i < 500; ++i) {
        size_t size1 = size_dist(size_gen);
        size_t size2 = size_dist(size_gen);
        size_t min_size = std::min(size1, size2);
        
        auto vec1 = generate_random_vector<T>(size1, seed + i + 400000);
        auto vec2 = generate_random_vector<T>(size2, seed + i + 500000);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        auto std_result = std::ranges::swap_ranges(vec1_copy.begin(), vec1_copy.end(), vec2_copy.begin(), vec2_copy.end());
        
        raze_assert(std::ranges::equal(vec1, vec1_copy));
        raze_assert(std::ranges::equal(vec2, vec2_copy));
        
        for (size_t j = 0; j < min_size; ++j) {
            raze_assert(vec1[j] == vec1_copy[j]);
            raze_assert(vec2[j] == vec2_copy[j]);
        }
        
        for (size_t j = min_size; j < size1; ++j) {
            raze_assert(vec1[j] == vec1_copy[j]);
        }
        
        for (size_t j = min_size; j < size2; ++j) {
            raze_assert(vec2[j] == vec2_copy[j]);
        }
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size1 = 100;
        size_t size2 = 50;
        
        auto vec1 = generate_random_vector<T>(size1, seed + i + 600000);
        auto vec2 = generate_random_vector<T>(size2, seed + i + 700000);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result.in1 == vec1.begin() + 50);
        raze_assert(simd_result.in2 == vec2.end());
        
        for (size_t j = 0; j < 50; ++j) {
            raze_assert(vec1[j] == vec2_copy[j]);
            raze_assert(vec2[j] == vec1_copy[j]);
        }
        
        for (size_t j = 50; j < 100; ++j) {
            raze_assert(vec1[j] == vec1_copy[j]);
        }
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size1 = 50;
        size_t size2 = 100;
        
        auto vec1 = generate_random_vector<T>(size1, seed + i + 800000);
        auto vec2 = generate_random_vector<T>(size2, seed + i + 900000);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result.in1 == vec1.end());
        raze_assert(simd_result.in2 == vec2.begin() + 50);
        
        for (size_t j = 0; j < 50; ++j) {
            raze_assert(vec1[j] == vec2_copy[j]);
            raze_assert(vec2[j] == vec1_copy[j]);
        }
        
        for (size_t j = 50; j < 100; ++j) {
            raze_assert(vec2[j] == vec2_copy[j]);
        }
    }
}

template <typename T>
void test_swap_ranges_edge_cases() {
    {
        std::vector<T> vec1;
        std::vector<T> vec2;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result.in1 == vec1.end());
        raze_assert(simd_result.in2 == vec2.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2;
        auto vec1_copy = vec1;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result.in1 == vec1.begin());
        raze_assert(simd_result.in2 == vec2.end());
        raze_assert(std::ranges::equal(vec1, vec1_copy));
    }
    
    {
        std::vector<T> vec1;
        std::vector<T> vec2 = {T(1), T(2), T(3)};
        auto vec2_copy = vec2;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(simd_result.in1 == vec1.end());
        raze_assert(simd_result.in2 == vec2.begin());
        raze_assert(std::ranges::equal(vec2, vec2_copy));
    }
    
    {
        std::vector<T> vec1 = {T(42)};
        std::vector<T> vec2 = {T(99)};
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(vec1[0] == T(99));
        raze_assert(vec2[0] == T(42));
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(10), T(20), T(30)};
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        auto simd_result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(vec1[0] == T(10));
        raze_assert(vec1[1] == T(20));
        raze_assert(vec1[2] == T(30));
        raze_assert(vec2[0] == T(1));
        raze_assert(vec2[1] == T(2));
        raze_assert(vec2[2] == T(3));
        raze_assert(simd_result.in1 == vec1.end());
        raze_assert(simd_result.in2 == vec2.end());
    }
    
    {
        std::vector<T> vec1(100, T(1));
        std::vector<T> vec2(100, T(2));
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        for (const auto& x : vec1) {
            raze_assert(x == T(2));
        }
        for (const auto& x : vec2) {
            raze_assert(x == T(1));
        }
    }
    
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256}) {
        std::vector<T> vec1(size, T(10));
        std::vector<T> vec2(size, T(20));
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        for (const auto& x : vec1) {
            raze_assert(x == T(20));
        }
        for (const auto& x : vec2) {
            raze_assert(x == T(10));
        }
    }
}

template <typename T>
void test_swap_ranges_return_value() {
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(10), T(20), T(30)};
        
        auto result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(result.in1 == vec1.end());
        raze_assert(result.in2 == vec2.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3), T(4), T(5)};
        std::vector<T> vec2 = {T(10), T(20)};
        
        auto result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(result.in1 == vec1.begin() + 2);
        raze_assert(result.in2 == vec2.end());
    }
    
    {
        std::vector<T> vec1;
        std::vector<T> vec2;
        
        auto result = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(result.in1 == vec1.end());
        raze_assert(result.in2 == vec2.end());
    }
    
    {
        std::vector<T> vec1 = {T(1), T(2), T(3)};
        std::vector<T> vec2 = {T(10), T(20), T(30)};
        
        auto [in1, in2] = raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(in1 == vec1.end());
        raze_assert(in2 == vec2.end());
    }
}

template <typename T>
void test_swap_ranges_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        auto vec1 = generate_random_vector<T>(size, seed + static_cast<unsigned>(size));
        auto vec2 = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + 1);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(std::ranges::equal(vec1, vec2_copy));
        raze_assert(std::ranges::equal(vec2, vec1_copy));
    }
}

template <typename T>
void test_swap_ranges_simd_boundaries(unsigned seed = 42) {
    for (size_t n : {0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257}) {
        size_t size = n + 100;
        auto vec1 = generate_random_vector<T>(size, seed + static_cast<unsigned>(n));
        auto vec2 = generate_random_vector<T>(size, seed + static_cast<unsigned>(n) + 1000);
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::swap_ranges(vec1.begin(), vec1.begin() + n, vec2.begin(), vec2.begin() + n);
        
        for (size_t j = 0; j < n; ++j) {
            raze_assert(vec1[j] == vec2_copy[j]);
            raze_assert(vec2[j] == vec1_copy[j]);
        }
        
        for (size_t j = n; j < size; ++j) {
            raze_assert(vec1[j] == vec1_copy[j]);
            raze_assert(vec2[j] == vec2_copy[j]);
        }
    }
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

void test_swap_ranges_struct(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        std::vector<Point> vec1(100);
        std::vector<Point> vec2(100);
        RandomGenerator<int> gen(seed + i + 100000);
        for (auto& p : vec1) {
            p.x = gen();
            p.y = gen();
        }
        for (auto& p : vec2) {
            p.x = gen();
            p.y = gen();
        }
        auto vec1_copy = vec1;
        auto vec2_copy = vec2;
        
        raze::algorithm::swap_ranges(vec1.begin(), vec1.end(), vec2.begin(), vec2.end());
        
        raze_assert(vec1 == vec2_copy);
        raze_assert(vec2 == vec1_copy);
    }
}

template <typename T>
void test_swap_ranges_list(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 200);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        auto list1 = generate_random_list<T>(size, seed + i + 200000);
        auto list2 = generate_random_list<T>(size, seed + i + 300000);
        auto lst1_copy = list1;
        auto lst2_copy = list2;
        
        raze::algorithm::swap_ranges(list1.begin(), list1.end(), list2.begin(), list2.end());
        
        raze_assert(std::ranges::equal(list1, lst2_copy));
        raze_assert(std::ranges::equal(list2, lst1_copy));
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size1 = 50 + (i % 50);
        size_t size2 = 30 + (i % 40);
        
        auto list1 = generate_random_list<T>(size1, seed + i + 400000);
        auto list2 = generate_random_list<T>(size2, seed + i + 500000);
        auto lst1_copy = list1;
        auto lst2_copy = list2;
        
        auto result = raze::algorithm::swap_ranges(list1.begin(), list1.end(), list2.begin(), list2.end());
        
        size_t min_size = std::min(size1, size2);
        auto it1 = list1.begin();
        auto it2 = list2.begin();
        auto it1_copy = lst1_copy.begin();
        auto it2_copy = lst2_copy.begin();
        
        for (size_t j = 0; j < min_size; ++j, ++it1, ++it2, ++it1_copy, ++it2_copy) {
            raze_assert(*it1 == *it2_copy);
            raze_assert(*it2 == *it1_copy);
        }
    }
}

template <typename T>
void test_swap_ranges_forward_list(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 200);
    
    for (int i = 0; i < 200; ++i) {
        size_t size = size_dist(size_gen);
        auto flst1 = generate_random_forward_list<T>(size, seed + i + 600000);
        auto flst2 = generate_random_forward_list<T>(size, seed + i + 700000);
        auto flst1_copy = flst1;
        auto flst2_copy = flst2;
        
        raze::algorithm::swap_ranges(flst1.begin(), flst1.end(), flst2.begin(), flst2.end());
        
        raze_assert(std::ranges::equal(flst1, flst2_copy));
        raze_assert(std::ranges::equal(flst2, flst1_copy));
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size1 = 50 + (i % 50);
        size_t size2 = 30 + (i % 40);
        
        auto flst1 = generate_random_forward_list<T>(size1, seed + i + 800000);
        auto flst2 = generate_random_forward_list<T>(size2, seed + i + 900000);
        auto flst1_copy = flst1;
        auto flst2_copy = flst2;
        
        auto result = raze::algorithm::swap_ranges(flst1.begin(), flst1.end(), flst2.begin(), flst2.end());
        
        size_t min_size = std::min(size1, size2);
        auto it1 = flst1.begin();
        auto it2 = flst2.begin();
        auto it1_copy = flst1_copy.begin();
        auto it2_copy = flst2_copy.begin();
        
        for (size_t j = 0; j < min_size; ++j, ++it1, ++it2, ++it1_copy, ++it2_copy) {
            raze_assert(*it1 == *it2_copy);
            raze_assert(*it2 == *it1_copy);
        }
    }
}

template <typename T>
void test_swap_ranges_mixed_containers(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(10, 200);
    
    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 1000000);
        auto lst = generate_random_list<T>(size, seed + i + 1100000);
        auto vec_copy = vec;
        auto lst_copy = lst;
        
        raze::algorithm::swap_ranges(vec.begin(), vec.end(), lst.begin(), lst.end());
        
        raze_assert(std::ranges::equal(vec, lst_copy));
        raze_assert(std::ranges::equal(lst, vec_copy));
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 1200000);
        auto flst = generate_random_forward_list<T>(size, seed + i + 1300000);
        auto vec_copy = vec;
        auto flst_copy = flst;
        
        raze::algorithm::swap_ranges(vec.begin(), vec.end(), flst.begin(), flst.end());
        
        raze_assert(std::ranges::equal(vec, flst_copy));
        raze_assert(std::ranges::equal(flst, vec_copy));
    }
    
    for (int i = 0; i < 100; ++i) {
        size_t size = size_dist(size_gen);
        auto lst = generate_random_list<T>(size, seed + i + 1400000);
        auto flst = generate_random_forward_list<T>(size, seed + i + 1500000);
        auto lst_copy = lst;
        auto flst_copy = flst;
        
        raze::algorithm::swap_ranges(lst.begin(), lst.end(), flst.begin(), flst.end());
        
        raze_assert(std::ranges::equal(lst, flst_copy));
        raze_assert(std::ranges::equal(flst, lst_copy));
    }
}

template <typename T>
void test_swap_ranges_overlapping(unsigned seed = 42) {
    for (int i = 0; i < 100; ++i) {
        size_t size = 100;
        auto vec = generate_random_vector<T>(size, seed + i + 1600000);
        auto vec_copy = vec;
        
        raze::algorithm::swap_ranges(vec.begin(), vec.begin() + 50, vec.begin() + 50, vec.end());
        
        for (size_t j = 0; j < 50; ++j) {
            raze_assert(vec[j] == vec_copy[j + 50]);
            raze_assert(vec[j + 50] == vec_copy[j]);
        }
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_swap_ranges_random<T>();
    test_swap_ranges_different_sizes<T>();
    test_swap_ranges_edge_cases<T>();
    test_swap_ranges_return_value<T>();
    test_swap_ranges_large_vectors<T>();
    test_swap_ranges_simd_boundaries<T>();
    test_swap_ranges_list<T>();
    test_swap_ranges_forward_list<T>();
    test_swap_ranges_mixed_containers<T>();
    test_swap_ranges_overlapping<T>();
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
    
    test_swap_ranges_struct();
    
    return 0;
}