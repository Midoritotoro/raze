#include <tests/vx/SimdTestTools.h>
#include <raze/vx/SimdDataparAlgorithms.h>

template<class Simd, class Mask, class T, size_t N, raze::uint8 _TernaryMask_>
void __test_ternarylogic_single_mask() {
    alignas(64) T arrA[N], arrB[N], arrC[N], arrSrc[N];
    for (size_t i = 0; i < N; ++i) {
        arrA[i]   = T(i + 1);
        arrB[i]   = T((i + 1) * 3);
        arrC[i]   = T((i + 1) * 7);
        arrSrc[i] = T(100 + i);
    }

    Simd a   = raze::vx::load<Simd>(arrA);
    Simd b   = raze::vx::load<Simd>(arrB);
    Simd c   = raze::vx::load<Simd>(arrC);
    Simd src = raze::vx::load<Simd>(arrSrc);

    Mask m = make_random_mask<Mask>();

    using _Reinterpret_type = typename raze::IntegerForSizeof<T>::Unsigned;

    auto scalar_eval = [](raze::uint8 mask, T A, T B, T C) -> _Reinterpret_type {
        _Reinterpret_type dst = 0;

        auto uA = std::bit_cast<_Reinterpret_type>(A);
        auto uB = std::bit_cast<_Reinterpret_type>(B);
        auto uC = std::bit_cast<_Reinterpret_type>(C);

        for (auto i = 0; i < (sizeof(T) * 8); ++i) {
            bool xa = (uA >> i) & 1;
            bool xb = (uB >> i) & 1;
            bool xc = (uC >> i) & 1;

            raze::uint8 idx =
                (raze::uint8(xa) << 2) |
                (raze::uint8(xb) << 1) |
                (raze::uint8(xc) << 0);

            bool bit = (mask >> idx) & 1;
            
            if (bit)
                raze::math::__bit_test_and_set(dst, i);
            else
                raze::math::__bit_test_and_reset(dst, i);
        }
        
        return dst;
    };

    using imm_t = std::integral_constant<raze::uint8, _TernaryMask_>;

    const auto run_tests = [a, b, c, scalar_eval, arrSrc, arrA, arrB, arrC](auto m, auto w, auto wz) {
        {
            auto r = raze::vx::ternarylogic(a, b, c, imm_t{});
            for (size_t i = 0; i < N; ++i) {
                _Reinterpret_type expected = scalar_eval(_TernaryMask_, arrA[i], arrB[i], arrC[i]);
                raze_assert(std::bit_cast<_Reinterpret_type>(T(r[i])) == expected);
            }
        }

        {
            auto r = raze::vx::ternarylogic(w, b, c, imm_t{});
            for (size_t i = 0; i < N; ++i) {
                auto expected = m[i] 
                    ? scalar_eval(_TernaryMask_, arrA[i], arrB[i], arrC[i]) 
                    : std::bit_cast<_Reinterpret_type>(T(arrSrc[i]));

                raze_assert(std::bit_cast<_Reinterpret_type>(T(r[i])) == expected);
            }
        }

        {
            auto r = raze::vx::ternarylogic(a, w, c, imm_t{});
            for (size_t i = 0; i < N; ++i) {
                auto expected = m[i] 
                    ? scalar_eval(_TernaryMask_, arrA[i], arrA[i], arrC[i]) 
                    : std::bit_cast<_Reinterpret_type>(T(arrSrc[i]));

                raze_assert(std::bit_cast<_Reinterpret_type>(T(r[i])) == expected);
            }
        }

        {
            auto r = raze::vx::ternarylogic(a, b, w, imm_t{});
            for (size_t i = 0; i < N; ++i) {
                auto expected = m[i] 
                    ? scalar_eval(_TernaryMask_, arrA[i], arrB[i], arrA[i]) 
                    : std::bit_cast<_Reinterpret_type>(T(arrSrc[i]));

                raze_assert(std::bit_cast<_Reinterpret_type>(T(r[i])) == expected);
            }
        }

        {
            auto r = raze::vx::ternarylogic(wz, b, c, imm_t{});
            for (size_t i = 0; i < N; ++i) {
                _Reinterpret_type expected = m[i] ? scalar_eval(_TernaryMask_, arrA[i], arrB[i], arrC[i]) : 0;
                raze_assert(std::bit_cast<_Reinterpret_type>(T(r[i])) == expected);
            }
        }

        {
            auto r = raze::vx::ternarylogic(a, wz, c, imm_t{});
            for (size_t i = 0; i < N; ++i) {
                _Reinterpret_type expected = m[i] ? scalar_eval(_TernaryMask_, arrA[i], arrA[i], arrC[i]) : 0;
                raze_assert(std::bit_cast<_Reinterpret_type>(T(r[i])) == expected);
            }
        }

        {
            auto r = raze::vx::ternarylogic(a, b, wz, imm_t{});
            for (size_t i = 0; i < N; ++i) {
                _Reinterpret_type expected = m[i] ? scalar_eval(_TernaryMask_, arrA[i], arrB[i], arrA[i]) : 0;
                raze_assert(std::bit_cast<_Reinterpret_type>(T(r[i])) == expected);
            }
        }
    };

    for (auto i = 0; i < std::min(int(std::pow(2, N)), 10000); ++i) {
        auto rnd_mask = make_random_mask<Mask>();

        auto w = raze::vx::where(a, src, rnd_mask);
        auto wz = raze::vx::where(a, rnd_mask);

        auto const_w = raze::vx::where(Simd(a), src, rnd_mask);
        auto const_wz = raze::vx::where(Simd(a), rnd_mask);

        run_tests(rnd_mask, w, wz);
        run_tests(rnd_mask, const_w, const_wz);
    }

}

template<class Simd, class Mask, class T, size_t N, std::size_t... I>
void __test_ternarylogic_all_masks_impl(std::index_sequence<I...>) {
    ( __test_ternarylogic_single_mask<Simd, Mask, T, N, static_cast<raze::uint8>(I)>(), ... );
}

template<class Simd, class Mask, class T, size_t N>
void test_ternarylogic_all_masks() {
    __test_ternarylogic_all_masks_impl<Simd, Mask, T, N>(std::make_index_sequence<256>{});
}

template <
    class           _Type_,
    raze::arch::ISA _ISA_,
    raze::uint32    _Width_>
struct ternarylogic_tests {
    using Simd = raze::vx::simd<_Type_, raze::vx::x86_runtime_abi<_ISA_, _Width_>>;
    using Mask = typename Simd::mask_type;
    using U = typename raze::IntegerForSizeof<_Type_>::Unsigned;
    static constexpr size_t N = Simd::size();

    void operator()() {
        test_ternarylogic_all_masks<Simd, Mask, _Type_, N>();
    }
};

template <
    template <class, raze::arch::ISA, raze::uint32> class _Function_,
    raze::arch::ISA  _ISA_,
    raze::uint32 _Width_>
void __ternarylogic_test_all_helper() {
    if (!raze::arch::ProcessorFeatures::isSupported<_ISA_>())
        return;

    raze::type_traits::invoke(_Function_<raze::int8, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<raze::int16, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<raze::int32, _ISA_, _Width_>());
    raze::type_traits::invoke(_Function_<raze::int64, _ISA_, _Width_>());
}

template <template <class, raze::arch::ISA, raze::uint32> class _Function_>
void ternarylogic_test_all() {
    __test_all_helper<_Function_, raze::arch::ISA::SSE2, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX2, 256>();

    __test_all_helper<_Function_, raze::arch::ISA::AVX512F, 512>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLF, 128>();
    __test_all_helper<_Function_, raze::arch::ISA::AVX512VLF, 256>();
}

int main() {
    ternarylogic_test_all<ternarylogic_tests>();
    return 0;
}
