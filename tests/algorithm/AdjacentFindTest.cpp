#define _USE_STD_VECTOR_ALGORITHMS 0
#include <cassert>
#include <vector>
#include <algorithm>
#include <ranges>
#include <iostream>
#include <random>
#include <cstdint>
#include <type_traits>
#include <cmath>
#include <cstdlib>
#include <functional>

#include <raze/algorithm/find/AdjacentFind.h>
#include <raze/math/Math.h>

template <class It>
void check_adjacent(It first, It last)
{
    auto std_res = std::ranges::adjacent_find(first, last);
    auto simd_res = raze::algorithm::adjacent_find(first, last);
    raze_assert(std_res == simd_res);
}

template <class It, class Pred>
void check_adjacent(It first, It last, Pred pred)
{
    auto std_res = std::ranges::adjacent_find(first, last, pred);
    auto simd_res = raze::algorithm::adjacent_find(first, last, pred);
    raze_assert(std_res == simd_res);
}

template <class It, class Pred, class Proj>
void check_adjacent(It first, It last, Pred pred, Proj proj)
{
    auto std_res = std::ranges::adjacent_find(first, last, pred, proj);
    auto simd_res = raze::algorithm::adjacent_find(first, last, pred, proj);
    raze_assert(std_res == simd_res);
}

template <class Rng>
void check_adjacent_range(Rng& rng)
{
    auto std_res = std::ranges::adjacent_find(rng);
    auto simd_res = raze::algorithm::adjacent_find(rng); 
    raze_assert(std_res == simd_res);
}

template <class Rng, class Pred>
void check_adjacent_range(Rng& rng, Pred pred)
{
    auto std_res = std::ranges::adjacent_find(rng, pred);
    auto simd_res = raze::algorithm::adjacent_find(rng, pred);
    raze_assert(std_res == simd_res);
}

template <class Rng, class Pred, class Proj>
void check_adjacent_range(Rng& rng, Pred pred, Proj proj)
{
    auto std_res = std::ranges::adjacent_find(rng, pred, proj);
    auto simd_res = raze::algorithm::adjacent_find(rng, pred, proj);
    raze_assert(std_res == simd_res);
}

struct Item {
    int id;
    int value;
};

template <class T>
void run_tests_for_type()
{
    {
        std::vector<T> v = {};
        check_adjacent(v.begin(), v.end());
        check_adjacent_range(v);
        
        check_adjacent(v.begin(), v.end(), std::greater<>{});
        check_adjacent_range(v, std::greater<>{});
    }
    {
        std::vector<T> v = { T(42) };
        check_adjacent(v.begin(), v.end());
        check_adjacent_range(v);
        
        check_adjacent(v.begin(), v.end(), std::less<>{});
        check_adjacent_range(v, std::less<>{});
    }

    {
        std::vector<T> v = { T(1), T(2), T(3), T(4), T(5) };
        check_adjacent(v.begin(), v.end());
        check_adjacent_range(v);
    }
    {
        std::vector<T> v = { T(1), T(1), T(2), T(3), T(4) };
        check_adjacent(v.begin(), v.end());
        check_adjacent_range(v);
    }
    {
        std::vector<T> v = { T(-1), T(2), T(-5), T(5), T(3)};
        auto proj = [](T x) { return raze::math::abs(x); };
        check_adjacent(v.begin(), v.end(), std::equal_to<>{}, proj);
        check_adjacent_range(v, std::equal_to<>{}, proj);
    }
    {
        std::vector<T> v = { T(1), T(3), T(2), T(4), T(5) };
        check_adjacent(v.begin(), v.end(), std::greater<>{});
        check_adjacent_range(v, std::greater<>{});
    }
    {
        std::vector<T> v = { T(10), T(20), T(21), T(30) };
        auto pred = [](T a, T b) { return raze::math::abs(a - b) < 2; };
        check_adjacent(v.begin(), v.end(), pred);
        check_adjacent_range(v, pred);
    }

    for (std::size_t n = 1; n < 200; ++n) {
        std::vector<T> v(n);
        for (std::size_t i = 0; i < n; ++i)
            v[i] = T(i % 5);
        check_adjacent(v.begin(), v.end());
        check_adjacent_range(v);
    }

    {
        const std::size_t N = 100'00;
        std::vector<T> v(N);
        for (std::size_t i = 0; i < N; ++i)
            v[i] = T(i % 13);
        check_adjacent(v.begin(), v.end());
        check_adjacent_range(v);
    }

    {
        std::mt19937_64 rng(123456);
        std::uniform_int_distribution<std::size_t> dist_length(0, 5000);
        std::uniform_int_distribution<int> dist_int(-1000, 1000);
        std::uniform_real_distribution<double> dist_fp(-1000.0, 1000.0);

        for (int iter = 0; iter < 100'00; ++iter) {
            const std::size_t N = dist_length(rng);
            std::vector<T> v(N);

            if constexpr (std::is_floating_point_v<T>) {
                for (std::size_t i = 0; i < N; ++i) v[i] = T(dist_fp(rng));
            } else {
                for (std::size_t i = 0; i < N; ++i) v[i] = T(dist_int(rng));
            }

            check_adjacent(v.begin(), v.end());
            check_adjacent_range(v);

            check_adjacent(v.begin(), v.end(), std::greater<>{});
            check_adjacent_range(v, std::greater<>{});

            if constexpr (std::is_floating_point_v<T>) {
                auto pred_close = [](T a, T b) { return raze::math::abs(a - b) < 5.0; };
                check_adjacent(v.begin(), v.end(), pred_close);
                check_adjacent_range(v, pred_close);
            } else {
                auto pred_close = [](T a, T b) { return raze::math::abs(a - b) < 5; };
                check_adjacent(v.begin(), v.end(), pred_close);
                check_adjacent_range(v, pred_close);
            }

            if constexpr (!std::is_unsigned_v<T>) {
                auto proj_abs = [](T x) { return raze::math::abs(x); };
                check_adjacent(v.begin(), v.end(), std::equal_to<>{}, proj_abs);
                check_adjacent_range(v, std::equal_to<>{}, proj_abs);
            }
        }
    }
}

void run_tests_for_struct()
{
    std::mt19937_64 rng(987654);
    std::uniform_int_distribution<std::size_t> dist_length(0, 5000);
    std::uniform_int_distribution<int> dist_val(0, 100);

    for (int iter = 0; iter < 10'00; ++iter) {
        const std::size_t N = dist_length(rng);
        std::vector<Item> v(N);
        
        for (std::size_t i = 0; i < N; ++i) {
            v[i] = { int(i % 10), dist_val(rng) };
        }

        auto get_id = [](const Item& it) { return it.id; };
        auto get_val = [](const Item& it) { return it.value; };

        check_adjacent(v.begin(), v.end(), std::equal_to<>{}, get_id);
        check_adjacent_range(v, std::equal_to<>{}, get_id);

        check_adjacent(v.begin(), v.end(), std::equal_to<>{}, get_val);
        check_adjacent_range(v, std::equal_to<>{}, get_val);

        auto pred_same_parity = [](int a, int b) { return (a % 2) == (b % 2); };
        check_adjacent(v.begin(), v.end(), pred_same_parity, get_val);
        check_adjacent_range(v, pred_same_parity, get_val);
        
        check_adjacent(v.begin(), v.end(), std::greater<>{}, get_id);
        check_adjacent_range(v, std::greater<>{}, get_id);
    }
}

int main() {
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
    
    run_tests_for_struct();
    
    return 0;
}