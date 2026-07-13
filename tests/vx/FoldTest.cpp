#include <tests/vx/SimdTestTools.h>
#include <raze/vx/Algorithm.h>

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::u32    _Width_>
struct fold_tests {
    template <raze::u64 _Size_>
    void test_size() {
        using Simd = raze::vx::simd<_Type_, raze::vx::runtime_abi<_ISA_, _Size_>>;
        using Mask = typename Simd::mask_type;
        static constexpr size_t N = Simd::size();

        alignas(64) _Type_ arr[N], fallback[N];

        std::iota(fallback, fallback + N, 1);

        for (size_t i = 0; i < N; ++i)
            arr[i] = _Type_((i % 3 == 0) ? -(i + 5) : (i + 1));

        Simd v = raze::vx::load<Simd>(arr);
        Simd fbk = raze::vx::load<Simd>(fallback);

        {
            auto hmin = raze::vx::fold(v, raze::vx::vertical_min);
            auto expected = (*std::min_element(arr, arr + N));
            raze_assert(hmin == expected);
        }

        {
            auto hmax = raze::vx::fold(v, raze::vx::vertical_max);
            auto expected = (*std::max_element(arr, arr + N));
            raze_assert(hmax == expected);
        }
        
        {
            auto hbroadcast = raze::vx::fold[raze::vx::broadcast](v, raze::vx::add);
            auto expected = std::accumulate(arr, arr + N, _Type_(0));

            for (size_t i = 0; i < N; ++i)
                raze_assert(hbroadcast[i] == expected);
        }

        if constexpr (!std::is_floating_point_v<_Type_>) {
            alignas(64) _Type_ arr_bits[N];

            std::fill(arr_bits, arr_bits + N, ~_Type_(0));
            Simd vbits = raze::vx::load<Simd>(arr_bits);
            auto hand = raze::vx::fold(vbits, raze::vx::bit_and);
            raze_assert(hand == _Type_(~_Type_(0)));

            arr_bits[0] = _Type_(0);
            vbits = raze::vx::load<Simd>(arr_bits);
            hand = raze::vx::fold(vbits, raze::vx::bit_and);
            raze_assert(hand == _Type_(0));
        }

        if constexpr (!std::is_floating_point_v<_Type_>) {
            alignas(64) _Type_ arr_bits[N];

            for (size_t i = 0; i < N; ++i)
                arr_bits[i] = _Type_(1 << i);

            Simd vbits = raze::vx::load<Simd>(arr_bits);
            auto hor = raze::vx::fold(vbits, raze::vx::bit_or);
            _Type_ expected = 0;
            for (size_t i = 0; i < N; ++i)
                expected |= arr_bits[i];
            raze_assert(hor == expected);
        }

        {
            auto hprod = raze::vx::fold(v, raze::vx::mul);
            _Type_ expected = 1;
            for (size_t i = 0; i < N; ++i)
                expected *= arr[i];
            raze_assert(hprod == expected);
            
            alignas(64) _Type_ arr_ones[N];

            std::fill(arr_ones, arr_ones + N, _Type_(1));
            v = raze::vx::load<Simd>(arr_ones);
            hprod = raze::vx::fold(v, raze::vx::mul);
            raze_assert(hprod == _Type_(1));
        }
    }

    void operator()() {
        test_size<_Width_ / (sizeof(_Type_) * 8)>();
        test_size<_Width_ / (sizeof(_Type_) * 8) * 2>();
        test_size<1>();
    }
};

int main() {
    test_all<fold_tests>();
    return 0;
}
