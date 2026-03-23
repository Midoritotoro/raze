#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>
#include <cstdint>
#include <type_traits>
#include <raze/algorithm/find/Count.h>


template <class T>
std::size_t raze_count(const T* data, std::size_t n, T value) {
    return raze::algorithm::count(data, data + n, value);
}

template <class T>
void check_single_case(const std::vector<T>& v, T value)
{
    std::size_t simd_res = raze_count(v.data(), v.size(), value);
    std::size_t std_res = std::count(v.begin(), v.end(), value);

    raze_assert(simd_res == std_res);
}

template <class T>
void run_tests_for_type()
{
    {
        std::vector<T> v = { 1,2,3,2,2,5,2,7 };
        check_single_case(v, T(2));
        check_single_case(v, T(5));
        check_single_case(v, T(42));
    }

    for (std::size_t n = 1; n < 200; ++n) {
        std::vector<T> v(n);
        for (std::size_t i = 0; i < n; ++i)
            v[i] = T(i % 7);

        for (int val = 0; val < 7; ++val)
            check_single_case(v, T(val));
    }

    {
        const std::size_t N = 1'000'000;
        std::vector<T> v(N);
        for (std::size_t i = 0; i < N; ++i)
            v[i] = T(i % 13);

        for (int val = 0; val < 13; ++val)
            check_single_case(v, T(val));
    }

    {
        std::mt19937_64 rng(123456);
        std::uniform_int_distribution<uint64_t> dist_int;
        std::uniform_real_distribution<double> dist_fp(-1000.0, 1000.0);

        const std::size_t N = 4096;
        std::vector<T> v(N);

        for (int iter = 0; iter < 1'000'000; ++iter) {
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

            std::size_t simd_res = raze_count(v.data(), N, pattern);
            std::size_t std_res = std::count(v.begin(), v.end(), pattern);

            raze_assert(simd_res == std_res);
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
