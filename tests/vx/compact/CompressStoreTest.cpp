#pragma once

#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>
#include <algorithm>

RAZE_TEST_NAMESPACE_BEGIN

namespace {
    template <class T>
    constexpr T make_value(size_t i) {
        if constexpr (std::is_floating_point_v<T>) return T(i) * T(0.5) - T(3);
        else return T(i + 1);
    }
    template <class T>
    constexpr T make_sentinel() { return T(-77); }

    template <class V>
    size_t reference_compress(const typename V::value_type* src,
                              typename V::value_type* out,
                              const typename V::mask_type& m) {
        size_t k = 0;
        for (size_t j = 0; j < V::size(); ++j)
            if (!m[j]) out[k++] = src[j];
        return k;
    }

    template <class T>
    bool bits_eq(T a, T b) { return rtts::detail::to_bits(a) == rtts::detail::to_bits(b); }
}

RTTS_CASE_TPL("raze::vx::compress_store", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V    = typename Simd::type;
    using T    = typename V::value_type;
    using Mask = typename V::mask_type;
    constexpr size_t N = V::size();
    constexpr size_t G = 8;                  
    const T SENT = make_sentinel<T>();

    alignas(std::hardware_constructive_interference_size) T src[N];
    for (size_t i = 0; i < N; ++i) src[i] = make_value<T>(i);
    const V v = raze::vx::load<V>(src);

    auto run_case = [&](const Mask& m, int mode, size_t offset) {
        alignas(std::hardware_constructive_interference_size) T buf[N + 2 * G];
        std::ranges::fill(buf, SENT);
        T* dst = buf + G + offset;

        T expected[N];
        std::ranges::fill(expected, SENT);
        const size_t k = reference_compress<V>(src, expected, m);

        T* end = nullptr;
        if      (mode == 1) end = raze::vx::compress_store[raze::vx::unsafe](dst, v, m);
        else if (mode == 2) end = raze::vx::compress_store[raze::vx::aligned](dst, v, m);
        else if (mode == 3) end = raze::vx::compress_store[raze::vx::aligned][raze::vx::unsafe](dst, v, m);
        else                end = raze::vx::compress_store(dst, v, m);

        RTTS_EXPECT(end == dst + k);
        for (size_t i = 0; i < k; ++i) RTTS_EXPECT(bits_eq(dst[i], expected[i]));
        if (mode == 0 || mode == 2)
            for (size_t i = k; i < N; ++i) RTTS_EXPECT(bits_eq(dst[i], SENT));
        for (size_t i = 0; i < G + offset; ++i)        RTTS_EXPECT(bits_eq(buf[i], SENT));
        for (size_t i = G + offset + N; i < N + 2*G; ++i) RTTS_EXPECT(bits_eq(buf[i], SENT));
    };

    auto run_all_modes = [&](const Mask& m) {
        run_case(m, 0, 0);  
        run_case(m, 0, 1);   
        run_case(m, 1, 0);    
        run_case(m, 1, 1);  
        run_case(m, 2, 0);    
        run_case(m, 3, 0);  
    };

    auto set_mask = [&](Mask& m, unsigned long long pattern) {
        for (size_t j = 0; j < N; ++j) m[j] = bool((pattern >> j) & 1);
    };

    {
        Mask m;
        set_mask(m, 0);                       run_all_modes(m);    
        set_mask(m, ~0ull);                   run_all_modes(m);  
        for (size_t b = 0; b < N; ++b) {     
            set_mask(m, 1ull << b);           run_all_modes(m);
            set_mask(m, ~(1ull << b));        run_all_modes(m);
        }
        for (size_t half = 0; half < 2; ++half) {
            for (size_t j = 0; j < N; ++j) m[j] = (j < N/2) == (half == 0);
            run_all_modes(m);
        }
        for (size_t j = 0; j < N; ++j) m[j] = (j % 2) == 0;  run_all_modes(m);
        for (size_t j = 0; j < N; ++j) m[j] = (j % 3) == 0;  run_all_modes(m);
    }

    if constexpr (N <= 16) {
        for (unsigned long long p = 0; p < (1ull << N); ++p) {
            Mask m; set_mask(m, p);
            run_case(m, 0, 0);
            run_case(m, 1, 0);
        }
    }

    for (int i = 0; i < 400; ++i) {
        Mask m = rtts::simd::make_random_mask<Mask>();
        run_all_modes(m);
    }

    {
        constexpr int R = 4;
        alignas(std::hardware_constructive_interference_size) T buf[N * R + 2 * G];
        std::ranges::fill(buf, SENT);
        T* cur = buf + G;
        T* exp_cur = buf + G;
        T expected[N * R];
        size_t total = 0;
        for (int r = 0; r < R; ++r) {
            Mask m = rtts::simd::make_random_mask<Mask>();
            T tmp[N];
            const size_t k = reference_compress<V>(src, tmp, m);
            std::copy(tmp, tmp + k, expected + total);
            total += k;
            cur = raze::vx::compress_store(cur, v, m);
        }
        RTTS_EXPECT(cur == buf + G + total);
        for (size_t i = 0; i < total; ++i) RTTS_EXPECT(bits_eq(buf[G + i], expected[i]));
        for (size_t i = G + total; i < N * R + 2 * G; ++i) RTTS_EXPECT(bits_eq(buf[i], SENT));
    }
};

RAZE_TEST_NAMESPACE_END