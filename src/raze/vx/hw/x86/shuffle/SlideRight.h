#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _VectorBytes_, sizetype _ElementBytes_, class _IdxType_>
consteval auto __make_slide_right_pshufb_table() noexcept {
    constexpr sizetype __num_elements = _VectorBytes_ / _ElementBytes_;
    constexpr sizetype __idx_bytes = sizeof(_IdxType_);
    constexpr sizetype __num_indices = _VectorBytes_ / __idx_bytes;

    constexpr sizetype __chunks_per_element = _ElementBytes_ / __idx_bytes;
    constexpr sizetype __num_shifts = __num_elements;

    std::array<std::array<_IdxType_, __num_indices>, __num_shifts> __table{};

    for (sizetype __shift = 0; __shift < __num_shifts; ++__shift) {
        for (sizetype __idx = 0; __idx < __num_indices; ++__idx) {
            sizetype __dst_elem = __idx / __chunks_per_element;
            sizetype __chunk_offset = __idx % __chunks_per_element;

            if (__dst_elem < __shift) __table[__shift][__idx] = static_cast<_IdxType_>(-1);
            else {
                sizetype __src_elem = __dst_elem - __shift;
                sizetype __src_chunk = __src_elem * __chunks_per_element + __chunk_offset;

                __table[__shift][__idx] = static_cast<_IdxType_>(__src_chunk);
            }
        }
    }

    return __table;
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_>
raze_nodiscard raze_no_stack_protector raze_always_inline auto __make_pshufb_slide_right_idx(_Intrin_, i32 __sh) noexcept {
    alignas(sizeof(_Intrin_)) static constexpr auto __table_u8 = __make_slide_right_pshufb_table<sizeof(_Intrin_), sizeof(_Type_), u8>();
    return _Rotate_indices<_Intrin_, u8>{ _Load<_ISA_, _Intrin_>()(__table_u8[__sh].data(), __aligned_policy{}) };
}

template <simd_type _Simd_, class _Int_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_right_fallback(const _Simd_& __x, _Int_ __sh) noexcept {
    alignas(sizeof(_Simd_)) typename _Simd_::value_type __arr[_Simd_::size() * 2];

    vx::__store[vx::aligned](__arr, _Simd_::zero());
    vx::__store[vx::aligned](__arr + _Simd_::size(), __x);

    return vx::__load<_Simd_>[vx::aligned](__arr + (_Simd_::size() - __sh));
}

template <intrin_type _Intrin_, class _Pattern_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Intrin_
__slide_right_native(_Intrin_ __x, _Pattern_ __p) noexcept
{
    constexpr auto __isa = abi_t<pattern_vector_t<_Pattern_>>::isa;
    using _Value_ = typename pattern_vector_t<_Pattern_>::value_type;

    constexpr auto __shift = __get_slide_right_shift(__p) == __shuffle_zero ? __p.size() : __get_slide_right_shift(__p);
    constexpr auto __shift_bytes = __shift * sizeof(_Value_);
    constexpr auto __size = __p.size();

    if constexpr (__shift == 0)
        return __x;

    if constexpr (sizeof(_Intrin_) == 16) return __as<_Intrin_>(_mm_slli_si128(__as<__m128i>(__x), __shift_bytes));
    else if constexpr (sizeof(_Intrin_) == 32 && __has_avx2_support_v<__isa>) {
        if constexpr (__has_avx512vl_support_v<__isa> && (__shift_bytes % 4) == 0) {
            return __as<_Intrin_>(_mm256_alignr_epi32(__as<__m256i>(__x), _mm256_setzero_si256(), (8 - __shift) & 7));
        }
        else {
            auto __low_part = _mm256_setzero_si256();
            auto __high_part = _mm256_setzero_si256();

            if constexpr (__shift_bytes >= sizeof(_Intrin_)) return _Zero<__isa, _Intrin_>()();
            else if constexpr (__shift_bytes == 0) return __x;
            else if constexpr (__shift_bytes < 16) {
                __low_part = __as<__m256i>(__x);
                __high_part = _mm256_inserti128_si256(__high_part, __as<__m128i>(__x), 1);
            }
            else if constexpr (__shift_bytes < 32) __low_part = _mm256_inserti128_si256(__low_part, __as<__m128i>(__x), 1);
            else return _Zero<__isa, _Intrin_>()();

            if constexpr ((__shift_bytes % 16) == 0) return __as<_Intrin_>(__low_part);
            return __as<_Intrin_>(_mm256_alignr_epi8(__low_part, __high_part, 16 - (__shift_bytes & 0xF)));
        }
    }
    else if constexpr (sizeof(_Intrin_) == 64) {
        if constexpr (__shift_bytes >= sizeof(_Intrin_)) return _Zero<__isa, _Intrin_>()();

        auto __low_part = _mm512_setzero_si512();
        auto __high_part = _mm512_setzero_si512();

        if constexpr (__shift_bytes == 0) return __x;
        else if constexpr (__shift_bytes < 16) {
            __low_part = __as<__m512i>(__x);
            __high_part = _mm512_maskz_shuffle_i64x2(0xFC, __as<__m512i>(__x), __as<__m512i>(__x), 0x90);
        }
        else if constexpr (__shift_bytes < 32) {
            __low_part = _mm512_maskz_shuffle_i64x2(0xFC, __as<__m512i>(__x), __as<__m512i>(__x), 0x90);
            __high_part = _mm512_maskz_shuffle_i64x2(0xF0, __as<__m512i>(__x), __as<__m512i>(__x), 0x40);
        }
        else if constexpr (__shift_bytes < 48) {
            __low_part = _mm512_maskz_shuffle_i64x2(0xF0, __as<__m512i>(__x), __as<__m512i>(__x), 0x40);
            __high_part = _mm512_maskz_shuffle_i64x2(0xC0, __as<__m512i>(__x), __as<__m512i>(__x), 0x00);
        }
        else if constexpr (__shift_bytes < 64) {
            __low_part = _mm512_maskz_shuffle_i64x2(0xC0, __as<__m512i>(__x), __as<__m512i>(__x), 0x00);
        }
        else return _Zero<__isa, _Intrin_>()();

        if constexpr ((__shift_bytes % 4) == 0)
            return __as<_Intrin_>(_mm512_alignr_epi32(__as<__m512i>(__x), _mm512_setzero_si512(), (16 - __shift_bytes) & 0xF));

        if constexpr (__has_avx512bw_support_v<__isa>) return __as<_Intrin_>(_mm512_alignr_epi8(__low_part, __high_part, 16 - (__shift_bytes & 0xF)));
        else {
            const auto __low = _mm256_alignr_epi8(__as<__m256i>(__low_part), __as<__m256i>(__high_part), 16 - (__shift_bytes & 0xF));
            const auto __high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(__as<__m512i>(__low_part), 1),
                _mm512_extracti64x4_epi64(__as<__m512i>(__high_part), 1), 16 - (__shift_bytes & 0xF));

            return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
        }
    }
    else {
        alignas(sizeof(_Intrin_)) _Value_ __arr[__p.size() * 2];

        _Store<__isa>()(__arr, _Zero<__isa, _Intrin_>()(), __aligned_policy{});
        _Store<__isa>()(__arr + __p.size(), __x, __aligned_policy{});

        return _Load<__isa, _Intrin_>()(__arr + (__p.size() - __shift), __aligned_policy{});
    }
}

template <class _Pattern_>
raze_nodiscard raze_no_stack_protector raze_always_inline pattern_vector_t<_Pattern_>
__slide_right(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept
{
    using _Simd_ = pattern_vector_t<_Pattern_>;

    if constexpr (__get_slide_right_shift(__p) == 0) return __x;
    else if constexpr (__get_slide_right_shift(__p) >= __p.size()) return _Simd_::zero();
    else if constexpr (native<_Simd_>) {
        auto __r = __x;

        auto& __storage = __r.template __get<0>();
        __storage = __slide_right_native(__storage_unwrap(__x.template __get<0>()), __p);

        return __r;
    }
    else return __slide_right_fallback(__x, __get_slide_right_shift(__p) == __shuffle_zero ? __p.size() : __get_slide_right_shift(__p));
}

template <simd_type _Simd_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_right(const _Simd_& __x, i32 __sh) noexcept {
    using _Abi_ = abi_t<_Simd_>;
    using _Value_ = typename _Simd_::value_type;

    static constexpr auto __select_isa = []() constexpr noexcept {
        if constexpr (sizeof(_Simd_) == 16 && __has_ssse3_support_v<_Abi_::isa>) return arch::ISA::SSSE3;
        else if constexpr (sizeof(_Simd_) == 32 && __has_avx2_support_v<_Abi_::isa>) return arch::ISA::AVX2;
    };

    if constexpr (native<_Simd_> && ((sizeof(_Simd_) == 16 && __has_ssse3_support_v<_Abi_::isa>)
        || (sizeof(_Simd_) == 32 && __has_avx2_support_v<_Abi_::isa>)))
    {
        constexpr auto __isa = __select_isa();

        using _Intrin_ = decltype(__storage_unwrap(__x.template __get<0>()));
        using _RetSlide = decltype(__make_pshufb_slide_right_idx<__isa, _Value_>(_Intrin_{}, __sh));
        using _IdxType = typename _RetSlide::index_type;

        using _Ret = decltype(__generic_shuffle_native<__isa, _IdxType>(_Intrin_{}, std::declval<_RetSlide>().data()));

        if constexpr (__is_fallback<_Ret>) return __slide_right_fallback(__x, __sh);
        else {
            auto __r = __x;
            auto& __storage = __r.template __get<0>();

            const auto __slide_indices = __make_pshufb_slide_right_idx<__isa, _Value_>(__storage_unwrap(__storage), __sh);
            using _IndexType = typename decltype(__slide_indices)::index_type;

            __storage = __generic_shuffle_native<__isa, _IndexType>(__storage_unwrap(__storage), __slide_indices.data());

            return __r;
        }
    }
    else return __slide_right_fallback(__x, __sh);
}

__RAZE_VX_NAMESPACE_END
