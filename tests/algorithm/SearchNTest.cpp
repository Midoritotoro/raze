#define _USE_STD_VECTOR_ALGORITHMS 0
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>
#include <cstdint>
#include <type_traits>
#include <raze/algorithm/find/SearchN.h>


template <class It, class Size, class T>
void check_search_n(It first, It last, Size count, const T& value)
{
    auto std_res = std::search_n(first, last, count, value);
    auto simd_res = raze::algorithm::search_n(first, last, count, value);
    raze_assert(std_res == simd_res);
}

template <class It, class Size, class T, class Pred>
void check_search_n(It first, It last, Size count, const T& value, Pred pred)
{
    auto std_res = std::search_n(first, last, count, value, pred);
    auto simd_res = raze::algorithm::search_n(first, last, count, value, pred);
    raze_assert(std_res == simd_res);
}

template <class T>
void run_tests_for_type()
{
    {
        std::vector<T> v = { 1,2,3,4,5,5,5,6 };
        check_search_n(v.begin(), v.end(), 1, T(5));
        check_search_n(v.begin(), v.end(), 2, T(5));
        check_search_n(v.begin(), v.end(), 3, T(5));
        check_search_n(v.begin(), v.end(), 4, T(5));
        check_search_n(v.begin(), v.end(), 1, T(42));
    }
    {
        std::vector<T> v = {};
        check_search_n(v.begin(), v.end(), 1, T(0));
    }
    {
        std::vector<T> v = { 42 };
        check_search_n(v.begin(), v.end(), 1, T(42));
        check_search_n(v.begin(), v.end(), 2, T(42));
    }
    {
        std::vector<T> v = { 1,1,1,1,1 };
        check_search_n(v.begin(), v.end(), 1, T(1));
        check_search_n(v.begin(), v.end(), 3, T(1));
        check_search_n(v.begin(), v.end(), 5, T(1));
        check_search_n(v.begin(), v.end(), 6, T(1));
    }
    {
        if constexpr (!std::is_floating_point_v<T>) {
            std::vector<T> v = { 1,2,3,4,5,6,7,8 };
            auto pred = [](T a, T b) { return (a % 2) == (b % 2); };
            check_search_n(v.begin(), v.end(), 2, T(3), pred);
        }
    }

    for (std::size_t n = 1; n < 200; ++n) {
        std::vector<T> v(n);
        for (std::size_t i = 0; i < n; ++i)
            v[i] = T(i % 7);

        for (int val = 0; val < 7; ++val) {
            check_search_n(v.begin(), v.end(), 1, T(val));
            check_search_n(v.begin(), v.end(), 2, T(val));
            check_search_n(v.begin(), v.end(), 3, T(val));
        }
    }

    {
        const std::size_t N = 1'000'000;
        std::vector<T> v(N);
        for (std::size_t i = 0; i < N; ++i)
            v[i] = T(i % 13);

        for (int val = 0; val < 13; ++val) {
            check_search_n(v.begin(), v.end(), 1, T(val));
            check_search_n(v.begin(), v.end(), 2, T(val));
            check_search_n(v.begin(), v.end(), 3, T(val));
        }
    }

    {
        std::mt19937_64 rng(123456);
        std::uniform_int_distribution<uint64_t> dist_length(0, 5000);
        std::uniform_int_distribution<uint64_t> dist_int;
        std::uniform_real_distribution<double> dist_fp(-1000.0, 1000.0);

        for (int iter = 0; iter < 1'000'000; ++iter) {
            const std::size_t N = dist_length(rng);
            std::vector<T> v(N);

            if constexpr (std::is_floating_point_v<T>) {
                for (std::size_t i = 0; i < N; ++i)
                    v[i] = T(dist_fp(rng));
            }
            else {
                for (std::size_t i = 0; i < N; ++i)
                    v[i] = T(dist_int(rng));
            }

            T pattern;
            if constexpr (std::is_floating_point_v<T>)
                pattern = T(dist_fp(rng));
            else
                pattern = T(dist_int(rng));

            check_search_n(v.begin(), v.end(), 1, pattern);
            check_search_n(v.begin(), v.end(), 2, pattern);
            check_search_n(v.begin(), v.end(), 3, pattern);
        }
    }
}

int main()
{
    run_tests_for_type<int8_t>();
    run_tests_for_type<uint8_t>();
    run_tests_for_type<int16_t>();
    run_tests_for_type<uint16_t>();
    run_tests_for_type<int32_t>();
    run_tests_for_type<uint32_t>();
    run_tests_for_type<int64_t>();
    run_tests_for_type<uint64_t>();
    run_tests_for_type<float>();
    run_tests_for_type<double>();
    return 0;
}
