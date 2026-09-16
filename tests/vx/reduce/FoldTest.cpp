#include <tests/rts/rts.h>
#include <raze/vx/Algorithm.h>

RAZE_TEST_NAMESPACE_BEGIN

RTTS_CASE_TPL("raze::vx::fold", rtts::simd::all_simd_infos)
<class Simd> (rtts::type<Simd>) {
    using V = typename Simd::type;
    using T = typename V::value_type;
    using Mask = typename V::mask_type;

    constexpr size_t N = V::size();

    T arr[N];
    T fallback[N];

    for (size_t i = 0; i < N; ++i) {
        fallback[i] = T(i + 1);
        arr[i] = T((i % 3 == 0) ? -(i + 5) : (i + 1));
    }

    V v = raze::vx::load<V>(arr);
    V fbk = raze::vx::load<V>(fallback);

    {
        auto result = raze::vx::fold(v, raze::vx::vertical_min);
        T expected = *std::min_element(arr, arr + N);
        RTTS_EXPECT(result == expected);
    }

    {
        auto result = raze::vx::fold(v, raze::vx::vertical_max);
        T expected = *std::max_element(arr, arr + N);
        RTTS_EXPECT(result == expected);
    }

    {
        auto result = raze::vx::fold[raze::vx::broadcast](v, raze::vx::add);
        T expected = std::accumulate(arr, arr + N, T(0));
        RTTS_ALL_VALIDATE_BITS(result, [&](auto) { return expected; });
    }

    if constexpr (!std::is_floating_point_v<T>) {
        T arr_bits[N];
        std::fill(arr_bits, arr_bits + N, T(~T(0)));
        V vbits = raze::vx::load<V>(arr_bits);

        auto result = raze::vx::fold(vbits, raze::vx::bit_and);
        RTTS_EXPECT(result == T(~T(0)));

        arr_bits[0] = T(0);
        vbits = raze::vx::load<V>(arr_bits);

        result = raze::vx::fold(vbits, raze::vx::bit_and);
        RTTS_EXPECT(result == T(0));
    }

    if constexpr (!std::is_floating_point_v<T>) {
        T arr_bits[N];

        for (size_t i = 0; i < N; ++i)
            arr_bits[i] = T(1) << i;

        V vbits = raze::vx::load<V>(arr_bits);
        auto result = raze::vx::fold(vbits, raze::vx::bit_or);
        T expected = T(0);

        for (size_t i = 0; i < N; ++i)
            expected |= arr_bits[i];

        RTTS_EXPECT(result == expected);
    }

    {
        auto result = raze::vx::fold(v, raze::vx::mul);
        T expected = T(1);

        for (size_t i = 0; i < N; ++i)
            expected *= arr[i];

        RTTS_EXPECT(result == expected);
    }

    {
        T arr_ones[N];
        std::fill(arr_ones, arr_ones + N, T(1));

        V ones = raze::vx::load<V>(arr_ones);
        auto result = raze::vx::fold(ones, raze::vx::mul);
        RTTS_EXPECT(result == T(1));
    }

    /*{
        auto result = raze::vx::fold[raze::vx::mirror](v, raze::vx::vertical_min);
        T expected = arr[0];

        for (size_t i = 1; i < N; ++i)
            expected = std::min(expected, arr[i]);

        RTTS_EXPECT(result == expected);
    }

    {
        auto result = raze::vx::fold[raze::vx::mirror](v, raze::vx::vertical_max);
        T expected = arr[0];

        for (size_t i = 1; i < N; ++i)
            expected = std::max(expected, arr[i]);

        RTTS_EXPECT(result == expected);
    }*/
};

RAZE_TEST_NAMESPACE_END
