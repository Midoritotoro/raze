#define _USE_STD_VECTOR_ALGORITHMS 0
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>
#include <cstdint>
#include <type_traits>

#include <raze/algorithm/find/AdjacentFind.h>

template <class It>
void check_adjacent(It first, It last)
{
    auto std_res = std::adjacent_find(first, last);
    auto simd_res = raze::algorithm::adjacent_find(first, last);
    raze_assert(std_res == simd_res);
}

template <class It, class Pred>
void check_adjacent(It first, It last, Pred pred)
{
    auto std_res = std::adjacent_find(first, last, pred);
    auto simd_res = raze::algorithm::adjacent_find(first, last, pred);
    raze_assert(std_res == simd_res);
}

template <class T>
void run_tests_for_type()
{
    {
        std::vector<T> v = { 1,2,3,4,5 };
        check_adjacent(v.begin(), v.end());
    }
    {
        std::vector<T> v = { 1,1,2,3,4 };
        check_adjacent(v.begin(), v.end());
    }
    {
        std::vector<T> v = { 5,4,3,2,1,1 };
        check_adjacent(v.begin(), v.end());
    }
    {
        std::vector<T> v = { 7,7,7,7 };
        check_adjacent(v.begin(), v.end());
    }
    {
        std::vector<T> v = {};
        check_adjacent(v.begin(), v.end());
    }
    {
        std::vector<T> v = { 42 };
        check_adjacent(v.begin(), v.end());
    }
    {
        if constexpr (!std::is_floating_point_v<T>) {
            std::vector<T> v = { 1,2,3,4,5,6 };
            auto pred = [](T a, T b) { return (a % 2) == (b % 2); };
            check_adjacent(v.begin(), v.end(), pred);
        }
    }
    {
        std::vector<T> v = { 1,3,5,7,9 };
        auto pred = [](T a, T b) { return true; };
        check_adjacent(v.begin(), v.end(), pred);
    }

    for (std::size_t n = 1; n < 200; ++n) {
        std::vector<T> v(n);
        for (std::size_t i = 0; i < n; ++i)
            v[i] = T(i % 5);
        if (n == 66)
            n = 66;
        check_adjacent(v.begin(), v.end());
    }

    {
        const std::size_t N = 1'000'000;
        std::vector<T> v(N);
        for (std::size_t i = 0; i < N; ++i)
            v[i] = T(i % 13);
        check_adjacent(v.begin(), v.end());
    }

    {
        std::mt19937_64 rng(123456);
        std::uniform_int_distribution<uint64_t> dist_length(0, 10000);
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
            check_adjacent(v.begin(), v.end());
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
