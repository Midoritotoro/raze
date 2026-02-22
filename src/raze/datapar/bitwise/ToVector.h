#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/shuffle/BroadcastZeros.h>

#include <raze/memory/PointerToIntegral.h>
#include <raze/math/IntegralTypesConversions.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
    class       _IntrinType_,
	class		_DesiredType_>
struct _Simd_to_vector;

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_to_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_> {
	template <class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline 
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>) {
            return __intrin_bitcast<_IntrinType_>(__mask);
        }
	    else if constexpr (sizeof(_DesiredType_) == 8) {
            const auto __broadcasted_mask = _mm_set1_epi8(static_cast<int8>(__mask));
            const auto __selected = _mm_and_si128(__broadcasted_mask, _mm_setr_epi32(1, 1, 2, 2));

            return __intrin_bitcast<_IntrinType_>(_mm_cmpgt_epi32(__selected, _mm_setzero_si128()));
        }
        else if constexpr (sizeof(_DesiredType_) == 4) {
            const auto __broadcasted_mask = _mm_set1_epi8(static_cast<int8>(__mask));
            const auto __selected = _mm_and_si128(__broadcasted_mask, _mm_setr_epi32(1, 2, 4, 8));

            return __intrin_bitcast<_IntrinType_>(_mm_cmpgt_epi32(__selected, _mm_setzero_si128()));
        }
        else if constexpr (sizeof(_DesiredType_) == 2) {
            const auto __broadcasted_mask = _mm_set1_epi8(static_cast<int8>(__mask));
            const auto __selected = _mm_and_si128(__broadcasted_mask, _mm_setr_epi32(0x00020001, 0x00080004, 0x00200010, 0x00800040));
        
            return __intrin_bitcast<_IntrinType_>(_mm_cmpgt_epi16(__selected, _mm_setzero_si128()));
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            const auto __not_mask = uint16(~__mask);

            const auto __broadcasted_low_mask = _mm_set1_epi8(static_cast<int8>(__not_mask));
            const auto __broadcasted_high_mask = _mm_set1_epi8(static_cast<int8>(__not_mask >> 8));

            const auto __vector_mask_low = _mm_setr_epi32(0x08040201, 0x80402010, 0, 0);
            const auto __vector_mask_high = _mm_setr_epi32(0, 0, 0x08040201, 0x80402010);

            const auto __selected_low = _mm_and_si128(__broadcasted_low_mask, __vector_mask_low);
            const auto __selected_high = _mm_and_si128(__broadcasted_high_mask, __vector_mask_high);

            const auto __combined = _mm_or_si128(__selected_low, __selected_high);
            return __intrin_bitcast<_IntrinType_>(_mm_cmpeq_epi8(__combined, _mm_setzero_si128()));
        }
	}
};

template <
    class _IntrinType_,
    class _DesiredType_> 
struct _Simd_to_vector<arch::ISA::SSSE3, 128, _IntrinType_, _DesiredType_>:
    _Simd_to_vector<arch::ISA::SSE3, 128, _IntrinType_, _DesiredType_> 
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline 
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept 
    {
        if constexpr (__is_intrin_type_v<_MaskType_>) {
            return __intrin_bitcast<_IntrinType_>(__mask);
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            const auto __select     = _mm_set1_epi64x(0x8040201008040201ull);
            const auto __shuffled   = _mm_shuffle_epi8(_mm_cvtsi32_si128(__mask), _mm_set_epi64x(0x0101010101010101ll, 0));

            return __intrin_bitcast<_IntrinType_>(_mm_cmpeq_epi8(_mm_and_si128(__shuffled, __select), __select));
        }
        else {
            return _Simd_to_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_>()(__mask);
        }
    }
};


template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_to_vector<arch::ISA::AVX2, 256, _IntrinType_, _DesiredType_> {
	template <class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline 
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>) {
            return __intrin_bitcast<_IntrinType_>(__mask);
        }
        else if constexpr (sizeof(_DesiredType_) == 8) {
            const auto __vector_mask = _mm256_set1_epi32(static_cast<int32>(__mask));
            const auto __selected = _mm256_and_si256(__vector_mask, _mm256_setr_epi32(1, 0, 2, 0, 4, 0, 8, 0));

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi64(__selected, _mm256_setzero_si256()));
        }
        else if constexpr (sizeof(_DesiredType_) == 4) {
            const auto __vector_mask = _mm256_set1_epi32(static_cast<int32>(__mask));
            const auto __selected = _mm256_and_si256(__vector_mask, _mm256_setr_epi32(1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80));

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi32(__selected, _mm256_setzero_si256()));
        }
        else if constexpr (sizeof(_DesiredType_) == 2) {
            const auto __vector_mask = _mm256_set1_epi16(static_cast<int16>(__mask));

            const auto __shuffled = _mm256_shuffle_epi8(__vector_mask, _mm256_setr_epi32(0, 0, 0, 0, 0x00010001, 0x00010001, 0x00010001, 0x00010001));
            const auto __select = _mm256_setr_epi32(0x00020001,0x00080004,0x00200010,0x00800040,0x00020001,0x00080004,0x00200010,0x00800040);

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpgt_epi16(_mm256_and_si256(__shuffled, __select), _mm256_setzero_si256()));
        }
        else if constexpr (sizeof(_DesiredType_) == 1) {
            const auto __vector_mask = _mm256_setr_epi32(__mask & 0xFFFF, 0, 0, 0, (__mask >> 16) & 0xFFFF, 0, 0, 0);

            const auto __select = _mm256_set1_epi64x(0x8040201008040201ull);
            const auto __shuffled = _mm256_shuffle_epi8(__vector_mask, _mm256_set_epi64x(0x0101010101010101ll, 0, 0x0101010101010101ll, 0));

            return __intrin_bitcast<_IntrinType_>(_mm256_cmpeq_epi8(_mm256_and_si256(__shuffled, __select), __select));
        }
	}
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_> {
	template <class _MaskType_>
	raze_nodiscard raze_static_operator raze_always_inline 
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>) {
            return __intrin_bitcast<_IntrinType_>(__mask);
        }
        else if constexpr (sizeof(_DesiredType_) == 8) {
            return __intrin_bitcast<_IntrinType_>(_mm512_maskz_mov_epi64(__mask, _mm512_set1_epi64(-1)));
        }
        else if constexpr (sizeof(_DesiredType_) == 4) {
            return __intrin_bitcast<_IntrinType_>(_mm512_maskz_mov_epi32(__mask, _mm512_set1_epi32(-1)));
        }
        else {
            using _MaskType = type_traits::__deduce_simd_mask_type<arch::ISA::AVX512F, _DesiredType_, 512>;
            using _HalfType = IntegerForSize<__constexpr_max<(sizeof(_MaskType) >> 1), 1>()>::Unsigned;

            constexpr auto __maximum    = math::__maximum_integral_limit<_HalfType>();
            constexpr auto __shift      = (sizeof(_MaskType) << 2);

            const auto __low = _Simd_to_vector<arch::ISA::AVX2, 256, __m256i, _DesiredType_>()(__mask & __maximum);
            const auto __high = _Simd_to_vector<arch::ISA::AVX2, 256, __m256i, _DesiredType_>()((__mask >> __shift));

            return __intrin_bitcast<_IntrinType_>(_mm512_inserti64x4(__intrin_bitcast<__m512i>(__low), __high, 1));
        }
	}
};

template <class _IntrinType_, class _DesiredType_> 
struct _Simd_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, _DesiredType_>:
    _Simd_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline 
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) == 1)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi8(__mask));

        else if constexpr (sizeof(_DesiredType_) == 2)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi16(__mask));

        else if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm512_maskz_mov_epi32(__mask, _mm512_set1_epi32(-1)));

        else if constexpr (sizeof(_DesiredType_) == 8)
            return __intrin_bitcast<_IntrinType_>(_mm512_maskz_mov_epi64(__mask, _mm512_set1_epi64(-1)));
    }
};

template <
    class _IntrinType_,
    class _DesiredType_> 
struct _Simd_to_vector<arch::ISA::AVX512BWDQ, 512, _IntrinType_, _DesiredType_>:
    _Simd_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline 
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) == 1)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi8(__mask));

        else if constexpr (sizeof(_DesiredType_) == 2)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi16(__mask));

        else if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi32(__mask));

        else if constexpr (sizeof(_DesiredType_) == 8)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi64(__mask));
    }
};

template <
    class _IntrinType_, 
    class _DesiredType_> 
struct _Simd_to_vector<arch::ISA::AVX512DQ, 512, _IntrinType_, _DesiredType_>:
    _Simd_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline 
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi32(__mask));

        else if constexpr (sizeof(_DesiredType_) == 8)
            return __intrin_bitcast<_IntrinType_>(_mm512_movm_epi64(__mask));

        else
            return _Simd_to_vector<arch::ISA::AVX512F, 512, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_, 
    class _DesiredType_>
struct _Simd_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_>:
    _Simd_to_vector<arch::ISA::AVX512VLF, 256, _IntrinType_, _DesiredType_> 
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) == 1)
            return __intrin_bitcast<_IntrinType_>(_mm256_movm_epi8(__mask));

        else if constexpr (sizeof(_DesiredType_) == 2)
            return __intrin_bitcast<_IntrinType_>(_mm256_movm_epi16(__mask));

        else
            return _Simd_to_vector<arch::ISA::AVX512VLF, 256, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_, 
    class _DesiredType_> 
struct _Simd_to_vector<arch::ISA::AVX512VLDQ, 256, _IntrinType_, _DesiredType_>:
    _Simd_to_vector<arch::ISA::AVX512VLF, 256, _IntrinType_, _DesiredType_> 
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm256_movm_epi32(__mask));

        else if constexpr (sizeof(_DesiredType_) == 8)
            return __intrin_bitcast<_IntrinType_>(_mm256_movm_epi64(__mask));

        else
            return _Simd_to_vector<arch::ISA::AVX2, 256, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_, 
    class _DesiredType_> 
struct _Simd_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_> :
    _Simd_to_vector<arch::ISA::AVX512VLF, 128, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) == 1)
            return __intrin_bitcast<_IntrinType_>(_mm_movm_epi8(__mask));

        else if constexpr (sizeof(_DesiredType_) == 2)
            return __intrin_bitcast<_IntrinType_>(_mm_movm_epi16(__mask));

        else
            return _Simd_to_vector<arch::ISA::SSE42, 128, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_,
    class _DesiredType_>
struct _Simd_to_vector<arch::ISA::AVX512VLDQ, 128, _IntrinType_, _DesiredType_>: 
    _Simd_to_vector<arch::ISA::AVX512VLF, 128, _IntrinType_, _DesiredType_> 
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) == 4)
            return __intrin_bitcast<_IntrinType_>(_mm_movm_epi32(__mask));

        else if constexpr (sizeof(_DesiredType_) == 8)
            return __intrin_bitcast<_IntrinType_>(_mm_movm_epi64(__mask));

        else
            return _Simd_to_vector<arch::ISA::SSE42, 128, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_, 
    class _DesiredType_>
struct _Simd_to_vector<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_, _DesiredType_> :
    _Simd_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) <= 2)
            return _Simd_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_>()(__mask);
        
        else
            return _Simd_to_vector<arch::ISA::AVX512VLDQ, 128, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <
    class _IntrinType_, 
    class _DesiredType_>
struct _Simd_to_vector<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_, _DesiredType_> :
    _Simd_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_>
{
    template <class _MaskType_>
    raze_nodiscard raze_static_operator raze_always_inline
        _IntrinType_ operator()(_MaskType_ __mask) raze_const_operator noexcept
    {
        if constexpr (__is_intrin_type_v<_MaskType_>)
            return __intrin_bitcast<_IntrinType_>(__mask);

        else if constexpr (sizeof(_DesiredType_) <= 2)
            return _Simd_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_>()(__mask);
        
        else
            return _Simd_to_vector<arch::ISA::AVX512VLDQ, 256, _IntrinType_, _DesiredType_>()(__mask);
    }
};

template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::SSE3, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::SSE2, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::SSE41, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::SSSE3, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::SSE42, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::SSE41, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX2, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::SSE42, 128, _IntrinType_, _DesiredType_> {};

template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMI, 512, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512BW, 512, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMI2, 512, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VBMI, 512, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512BWDQ, 512, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMI2DQ, 512, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VBMIDQ, 512, _IntrinType_, _DesiredType_> {};

template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VLF, 256, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX2, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMIVL, 256, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VLBW, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMI2VL, 256, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VBMIVL, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VLBWDQ, 256, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMI2VLDQ, 256, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VBMIVLDQ, 256, _IntrinType_, _DesiredType_> {};

template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VLF, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX2, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMIVL, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VLBW, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMI2VL, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VBMIVL, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VLBWDQ, 128, _IntrinType_, _DesiredType_> {};
template <class _IntrinType_, class _DesiredType_> struct _Simd_to_vector<arch::ISA::AVX512VBMI2VLDQ, 128, _IntrinType_, _DesiredType_> : _Simd_to_vector<arch::ISA::AVX512VBMIVLDQ, 128, _IntrinType_, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
