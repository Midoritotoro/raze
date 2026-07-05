#include <raze/algorithm/order/Rotate.h>

#include <vector>
#include <list>
#include <forward_list>
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

template <typename T>
bool equal_forward_lists(const std::forward_list<T>& a, const std::forward_list<T>& b) {
    auto it_a = a.begin();
    auto it_b = b.begin();
    while (it_a != a.end() && it_b != b.end()) {
        if (*it_a != *it_b) return false;
        ++it_a; ++it_b;
    }
    return it_a == a.end() && it_b == b.end();
}

template <typename T>
void test_rotate_random(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    std::uniform_int_distribution<int> mid_dist(0, 1000);

    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i);
        auto vec_copy = vec;

        size_t mid = std::min(static_cast<size_t>(mid_dist(size_gen)), size);

        auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin() + mid, vec.end());
        auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin() + mid, vec_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }
}

template <typename T>
void test_rotate_ranges(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 1000);
    std::uniform_int_distribution<int> mid_dist(0, 1000);

    for (int i = 0; i < 10000; ++i) {
        size_t size = size_dist(size_gen);
        auto vec = generate_random_vector<T>(size, seed + i + 30000);
        auto vec_copy = vec;

        size_t mid = std::min(static_cast<size_t>(mid_dist(size_gen)), size);

        auto simd_res = raze::algorithm::rotate(vec, vec.begin() + mid);
        auto std_res = std::ranges::rotate(vec_copy, vec_copy.begin() + mid);

        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }
}

template <typename T>
void test_rotate_edge_cases() {
    {
        std::vector<T> vec;
        auto vec_copy = vec;
        auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin(), vec.end());
        auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin(), vec_copy.end());
        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }

    {
        std::vector<T> vec = {T(42)};
        auto vec_copy = vec;
        auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin(), vec.end());
        auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin(), vec_copy.end());
        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }

    {
        std::vector<T> vec = {T(1), T(2), T(3), T(4)};
        auto vec_copy = vec;
        auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin(), vec.end());
        auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin(), vec_copy.end());
        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }

    {
        std::vector<T> vec = {T(1), T(2), T(3), T(4)};
        auto vec_copy = vec;
        auto simd_res = raze::algorithm::rotate(vec.begin(), vec.end(), vec.end());
        auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.end(), vec_copy.end());
        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }

    {
        std::vector<T> vec(100, T(7));
        auto vec_copy = vec;
        auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin() + 50, vec.end());
        auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin() + 50, vec_copy.end());
        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }
}

template <typename T>
void test_rotate_simd_boundaries(unsigned seed = 42) {
    for (size_t size : {1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512}) {
        for (size_t mid : { size_t(0), size_t(1), size_t(size / 2), size_t(size - 1), size_t(size) }) {
            if (mid <= size) {
                auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + static_cast<unsigned>(mid));
                auto vec_copy = vec;

                auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin() + mid, vec.end());
                auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin() + mid, vec_copy.end());

                raze_assert(vec == vec_copy);
                raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
                raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
            }
        }
    }
}

template <typename T>
void test_rotate_large_vectors(unsigned seed = 42) {
    for (size_t size : {10000, 50000, 100000}) {
        for (size_t mid : {size_t(0), size_t{ size / 4 }, size_t{ size / 2 }, size_t{ 3 * size / 4 }, size_t(size) }) {
            auto vec = generate_random_vector<T>(size, seed + static_cast<unsigned>(size) + static_cast<unsigned>(mid));
            auto vec_copy = vec;

            auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin() + mid, vec.end());
            auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin() + mid, vec_copy.end());

            raze_assert(vec == vec_copy);
            raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
            raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
        }
    }
}

template <typename T>
void test_rotate_list(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 500);
    std::uniform_int_distribution<int> mid_dist(0, 500);

    for (int i = 0; i < 2000; ++i) {
        size_t size = size_dist(size_gen);
        auto lst = generate_random_list<T>(size, seed + i + 100000);
        auto lst_copy = lst;

        size_t mid = std::min(static_cast<size_t>(mid_dist(size_gen)), size);

        auto it_mid = lst.begin();
        std::advance(it_mid, mid);
        auto it_mid_copy = lst_copy.begin();
        std::advance(it_mid_copy, mid);

        auto simd_res = raze::algorithm::rotate(lst.begin(), it_mid, lst.end());
        auto std_res = std::ranges::rotate(lst_copy.begin(), it_mid_copy, lst_copy.end());

        raze_assert(lst == lst_copy);
    }
}

template <typename T>
void test_rotate_forward_list(unsigned seed = 42) {
    std::mt19937 size_gen(seed + 1);
    std::uniform_int_distribution<int> size_dist(0, 500);
    std::uniform_int_distribution<int> mid_dist(0, 500);

    for (int i = 0; i < 2000; ++i) {
        size_t size = size_dist(size_gen);
        auto flst = generate_random_forward_list<T>(size, seed + i + 200000);
        auto flst_copy = flst;

        size_t mid = std::min(static_cast<size_t>(mid_dist(size_gen)), size);

        auto it_mid = flst.before_begin();
        auto it_mid_copy = flst_copy.before_begin();
        for (size_t j = 0; j < mid; ++j) {
            ++it_mid;
            ++it_mid_copy;
        }

        raze::algorithm::rotate(flst.begin(), 
            [&]() { 
                auto it = flst.begin(); 
                std::advance(it, mid); 
                return it; 
            }(), 
            flst.end());
            
        std::ranges::rotate(flst_copy.begin(), 
            [&]() { 
                auto it = flst_copy.begin(); 
                std::advance(it, mid); 
                return it; 
            }(), 
            flst_copy.end());

        raze_assert(equal_forward_lists(flst, flst_copy));
    }
}

template <typename T, std::size_t N>
void test_rotate_array_sorted() {
    std::array<T, N> arr{};
    for (std::size_t i = 0; i < N; ++i) arr[i] = static_cast<T>(i);
    auto arr_copy = arr;

    constexpr std::size_t mid = N / 2;
    
    auto simd_res = raze::algorithm::rotate(arr.begin(), arr.begin() + mid, arr.end());
    auto std_res = std::ranges::rotate(arr_copy.begin(), arr_copy.begin() + mid, arr_copy.end());

    raze_assert(arr == arr_copy);
    raze_assert((arr.end() - simd_res.begin()) == (arr_copy.end() - std_res.begin()));
    raze_assert((arr.end() - simd_res.end()) == (arr_copy.end() - std_res.end()));
}

template <typename T, std::size_t N>
void test_rotate_array_unsorted() {
    std::array<T, N> arr{};
    RandomGenerator<T> gen(42);
    for (std::size_t i = 0; i < N; ++i) arr[i] = gen();
    auto arr_copy = arr;

    constexpr std::size_t mid = N > 0 ? N / 3 : 0;

    auto simd_res = raze::algorithm::rotate(arr.begin(), arr.begin() + mid, arr.end());
    auto std_res = std::ranges::rotate(arr_copy.begin(), arr_copy.begin() + mid, arr_copy.end());

    raze_assert(arr == arr_copy);
    raze_assert((arr.end() - simd_res.begin()) == (arr_copy.end() - std_res.begin()));
    raze_assert((arr.end() - simd_res.end()) == (arr_copy.end() - std_res.end()));
}

template <typename T, std::size_t N>
void test_rotate_array_ranges() {
    std::array<T, N> arr{};
    RandomGenerator<T> gen(123);
    for (std::size_t i = 0; i < N; ++i) arr[i] = gen();
    auto arr_copy = arr;

    constexpr std::size_t mid = N > 0 ? N / 2 : 0;

    auto simd_res = raze::algorithm::rotate(arr, arr.begin() + mid);
    auto std_res = std::ranges::rotate(arr_copy, arr_copy.begin() + mid);

    raze_assert(arr == arr_copy);
    raze_assert((arr.end() - simd_res.begin()) == (arr_copy.end() - std_res.begin()));
    raze_assert((arr.end() - simd_res.end()) == (arr_copy.end() - std_res.end()));
}

template <typename T>
void test_rotate_arrays_compile_time_sizes() {
    test_rotate_array_sorted<T, 0>();
    test_rotate_array_sorted<T, 1>();
    test_rotate_array_sorted<T, 4>();
    test_rotate_array_sorted<T, 8>();
    test_rotate_array_sorted<T, 16>();
    test_rotate_array_sorted<T, 32>();
    test_rotate_array_sorted<T, 64>();
    test_rotate_array_sorted<T, 128>();
    test_rotate_array_sorted<T, 256>();

    test_rotate_array_unsorted<T, 4>();
    test_rotate_array_unsorted<T, 8>();
    test_rotate_array_unsorted<T, 16>();
    test_rotate_array_unsorted<T, 32>();
    test_rotate_array_unsorted<T, 64>();
    test_rotate_array_unsorted<T, 128>();
    test_rotate_array_unsorted<T, 256>();

    test_rotate_array_ranges<T, 16>();
    test_rotate_array_ranges<T, 64>();
    test_rotate_array_ranges<T, 128>();
}

struct Point {
    int x, y;
    bool operator==(const Point&) const = default;
};

template <typename T>
void test_rotate_struct(unsigned seed = 42) {
    for (int i = 0; i < 1000; ++i) {
        std::vector<Point> vec(100);
        RandomGenerator<int> gen(seed + i + 400000);
        for (auto& p : vec) { p.x = gen(); p.y = gen(); }
        auto vec_copy = vec;

        auto simd_res = raze::algorithm::rotate(vec.begin(), vec.begin() + 50, vec.end());
        auto std_res = std::ranges::rotate(vec_copy.begin(), vec_copy.begin() + 50, vec_copy.end());

        raze_assert(vec == vec_copy);
        raze_assert((vec.end() - simd_res.begin()) == (vec_copy.end() - std_res.begin()));
        raze_assert((vec.end() - simd_res.end()) == (vec_copy.end() - std_res.end()));
    }
}

template <typename T>
void run_all_tests_for_type() {
    test_rotate_random<T>();
    test_rotate_ranges<T>();
    test_rotate_edge_cases<T>();
    test_rotate_simd_boundaries<T>();
    test_rotate_large_vectors<T>();
    test_rotate_list<T>();
    test_rotate_forward_list<T>();
    test_rotate_arrays_compile_time_sizes<T>();
    test_rotate_struct<T>();
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

    test_rotate_struct<int>();

    return 0;
}