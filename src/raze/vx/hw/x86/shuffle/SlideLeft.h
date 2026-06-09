#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _VectorBytes_, sizetype _ElementBytes_, class _IdxType_>
consteval auto __make_slide_left_pshufb_table() noexcept {
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
            sizetype __src_elem = __dst_elem + __shift;

            if (__src_elem >= __num_elements) {
                if constexpr (sizeof(_IdxType_) == 1) __table[__shift][__idx] = static_cast<_IdxType_>(0x80);
                else __table[__shift][__idx] = static_cast<_IdxType_>(-1);
            }
            else {
                sizetype __src_chunk = __src_elem * __chunks_per_element + __chunk_offset;
                __table[__shift][__idx] = static_cast<_IdxType_>(__src_chunk);
            }
        }
    }

    return __table;
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_>
raze_nodiscard raze_no_stack_protector raze_always_inline auto __make_pshufb_slide_left_idx(_Intrin_, i32 __sh) noexcept {
    using _IdxType = typename IntegerForSizeof<_Type_>::Unsigned;
    alignas(64) static constexpr auto __table_u8 = __make_slide_left_pshufb_table<sizeof(_Intrin_), sizeof(_Type_), u8>();
    return _Rotate_indices<_Intrin_, u8> { _Load<_ISA_, _Intrin_>()(__table_u8[__sh].data(), __aligned_policy{}) };
}

template <simd_type _Simd_, class _Int_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_left_fallback(const _Simd_& __x, _Int_ __sh) noexcept {
    alignas(64) typename _Simd_::value_type __arr[_Simd_::size() * 2];

    vx::__store[vx::aligned](__arr, __x);
    vx::__store[vx::aligned](__arr + _Simd_::size(), _Simd_::zero());

    return vx::__load<_Simd_>[vx::aligned](__arr + __sh);
}

template <intrin_type _Intrin_, class _Pattern_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Intrin_ 
__slide_left_native(_Intrin_ __x, _Pattern_ __p) noexcept 
{
    constexpr auto __isa = abi_t<pattern_vector_t<_Pattern_>>::isa;
    using _Value_ = typename pattern_vector_t<_Pattern_>::value_type;

    constexpr auto __shift = __get_slide_left_shift(__p);
    constexpr auto __size = __p.size();

    if constexpr (__shift == 0)
        return __x;

    if constexpr (sizeof(_Intrin_) == 16) return __as<_Intrin_>(_mm_slli_si128(__as<__m128i>(__x), __shift * sizeof(_Value_)));
    else if constexpr (sizeof(_Intrin_) == 32 && __has_avx2_support_v<__isa>) {
        if constexpr (__shift > (__size / 2)) {
            const auto __y = _mm256_slli_si256(__as<__m256i>(__x), (__shift - (__size / 2)) * sizeof(_Value_));
            return __as<_Intrin_>(_mm256_permute2x128_si256(__y, __y, 0x28));
        }
        else if constexpr (__shift == (__size / 2)) {
            return __as<_Intrin_>(_mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x28));
        }

        const auto __y = _mm256_slli_si256(__as<__m256i>(__x), __shift * sizeof(_Value_));
        const auto __z = _mm256_srli_si256(__as<__m256i>(__x), 16 - (__shift * sizeof(_Value_)));
        const auto __w = _mm256_permute2x128_si256(__z, __z, 0x28);

        return __as<_Intrin_>(_mm256_or_si256(__y, __w));
    }
    else if constexpr (sizeof(_Intrin_) == 64) {

    }
}

template <class _Pattern_>
raze_nodiscard raze_no_stack_protector raze_always_inline pattern_vector_t<_Pattern_> 
__slide_left(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept 
{
    using _Simd_ = pattern_vector_t<_Pattern_>;

    if constexpr (native<_Simd_>) {
        auto __r = __x;

        auto& __storage = __r.template __get<0>();
        __storage = __slide_left_native(__storage_unwrap(__x.template __get<0>()), __p);

        return __r;
    }
    else return __slide_left_fallback(__x, __get_slide_left_shift(__p));
}

template <simd_type _Simd_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_left(const _Simd_& __x, i32 __sh) noexcept {
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
        using _RetRotate = decltype(__make_pshufb_slide_left_idx<__isa, _Value_>(_Intrin_{}, __sh));
        using _IdxType = typename _RetRotate::index_type;

        using _Ret = decltype(__generic_shuffle_native<__isa, _IdxType>(_Intrin_{}, std::declval<_RetRotate>().data()));
		
		if constexpr (__is_fallback<_Ret>) return __rotate_left_fallback(__x, __sh);
		else {
			auto __r = __x;
			auto& __storage = __r.template __get<0>();

            const auto __rotate_indices = __make_pshufb_slide_left_idx<__isa, _Value_>(__storage_unwrap(__r.template __get<0>()), __sh);
            using _IndexType = typename decltype(__rotate_indices)::index_type;

			__storage = __generic_shuffle_native<__isa, _IndexType>(__storage_unwrap(__storage), __rotate_indices.data());
			
			return __r;
		}
	}
	else return __slide_left_fallback(__x, __sh);
}

__RAZE_VX_NAMESPACE_END
