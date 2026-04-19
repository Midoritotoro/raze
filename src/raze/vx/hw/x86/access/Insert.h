#pragma once 

#include <src/raze/vx/hw/x86/construct/Broadcast.h>

#include <raze/memory/PointerToIntegral.h>
#include <src/raze/vx/hw/x86/access/InsertMask.h>

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_>
struct _Insert;

template <>
struct _Insert<arch::ISA::SSE2, 128> {
	template <
		class _IntrinType_,
		class _Type_>
	raze_static_operator raze_always_inline void operator()(
		_IntrinType_&	__vector, 
		uint8			__index,
		_Type_	__value) raze_const_operator noexcept 
	{
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
#if defined(raze_processor_x86_64)
            auto __vector_value = _mm_cvtsi64_si128(memory::pointer_to_integral(__value));
#else
            union {
                __m128i __vector;
                int64   __number;
            } __convert;

            __convert.__number = __value;
            auto __vector_value = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(&__convert.__vector));
#endif // defined(raze_processor_x86_64)
            __vector = (__index == 0)
                ? __as<_IntrinType_>(_mm_unpackhi_epi64(
                    _mm_unpacklo_epi64(__vector_value, __vector_value), __as<__m128i>(__vector)))
                : __as<_IntrinType_>(_mm_unpacklo_epi64(
                    __as<__m128i>(__vector), __vector_value));
        }
        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
            switch (__index) {
                case 0:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 0));
                    break;
                case 1:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 1));
                    break;
                case 2:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 2));
                    break;
                case 3:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 3));
                    break;
                case 4:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 4));
                    break;
                case 5:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 5));
                    break;
                case 6:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 6));
                    break;
                default:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 7));
                    break;
            }
        }
        else if constexpr (__is_pd_v<_Type_>) {
            const auto __broadcasted = _mm_set_sd(__value);

            __vector = (__index == 0)
                ? _mm_shuffle_pd(__broadcasted, __as<__m128d>(__vector), 2)
                : _mm_shuffle_pd(__as<__m128d>(__vector), __broadcasted, 0);
        }
        else {
            const auto __mask = __simd_make_insert_mask<_IntrinType_, typename IntegerForSizeof<_Type_>::Unsigned>();

            const auto __broadcasted = _Broadcast<arch::ISA::SSE2, 128, _IntrinType_>()(memory::pointer_to_integral(__value));
            const auto __insert_mask = _Load<arch::ISA::SSE2, 128, _IntrinType_>()(__mask.__array + __mask.__offset - __index);

            __vector = _Select<arch::ISA::SSE2, 128, _Type_>()(__broadcasted, __vector, __insert_mask);
        }
	}
};

template <> struct _Insert<arch::ISA::SSE3, 128> : _Insert<arch::ISA::SSE2, 128> {};
template <> struct _Insert<arch::ISA::SSSE3, 128> : _Insert<arch::ISA::SSE3, 128> {};


template <> 
struct _Insert<arch::ISA::SSE41, 128>: 
    _Insert<arch::ISA::SSSE3, 128> 
{
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
            auto __qword_value = memory::pointer_to_integral(__value);

            switch (__index) {
                case 0:
                    __vector = __as<_IntrinType_>(_mm_insert_epi64(
                        __as<__m128i>(__vector), __qword_value, 0));
                    break;
                default:
                    __vector = __as<_IntrinType_>(_mm_insert_epi64(
                        __as<__m128i>(__vector), __qword_value, 1));
                    break;
            }
        }
        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
            auto __dword_value = memory::pointer_to_integral(__value);

            switch (__index) {
                case 0:
                    __vector = __as<_IntrinType_>(_mm_insert_epi32(
                        __as<__m128i>(__vector), __dword_value, 0));
                    break;
                case 1:
                    __vector = __as<_IntrinType_>(_mm_insert_epi32(
                        __as<__m128i>(__vector), __dword_value, 1));
                    break;
                case 2:
                    __vector = __as<_IntrinType_>(_mm_insert_epi32(
                        __as<__m128i>(__vector), __dword_value, 2));
                    break;
                default:
                    __vector = __as<_IntrinType_>(_mm_insert_epi32(
                        __as<__m128i>(__vector), __dword_value, 3));
                    break;
            }
        }
        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
            switch (__index) {
                case 0:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 0));
                    break;
                case 1:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 1));
                    break;
                case 2:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 2));
                    break;
                case 3:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 3));
                    break;
                case 4:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 4));
                    break;
                case 5:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 5));
                    break;
                case 6:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 6));
                    break;
                default:
                    __vector = __as<_IntrinType_>(_mm_insert_epi16(
                        __as<__m128i>(__vector), __value, 7));
                    break;
            }
        }
        else {
            return _Insert<arch::ISA::SSE2, 128>()(__vector, __index, __value);
        }
    }
};

template <> struct _Insert<arch::ISA::SSE42, 128> : _Insert<arch::ISA::SSE41, 128> {};
template <> struct _Insert<arch::ISA::AVX, 128> : _Insert<arch::ISA::SSE42, 128> {};
template <> struct _Insert<arch::ISA::FMA3, 128> : _Insert<arch::ISA::SSE42, 128> {};
template <> struct _Insert<arch::ISA::AVX2, 128> : _Insert<arch::ISA::AVX, 128> {};
template <> struct _Insert<arch::ISA::AVX2FMA3, 128> : _Insert<arch::ISA::AVX2, 128> {};

template <> 
struct _Insert<arch::ISA::AVX512VLF, 128>:
    _Insert<arch::ISA::AVX2, 128>
{
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm_mask_set1_epi64(
                __as<__m128i>(__vector),
                static_cast<uint8>(uint8(1) << __index),
                memory::pointer_to_integral(__value)));
        }
        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm_mask_set1_epi32(
                __as<__m128i>(__vector), 
                static_cast<uint8>(uint8(1) << __index),
                memory::pointer_to_integral(__value)));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm_mask_broadcastss_ps(
                __as<__m128>(__vector),
                static_cast<uint8>(uint8(1) << __index),
                _mm_set_ss(__value)));
        }
        else {
            const auto __mask = __simd_make_insert_mask<_IntrinType_, typename IntegerForSizeof<_Type_>::Unsigned>();

            const auto __broadcasted = _Broadcast<arch::ISA::AVX512VLF, 128, _IntrinType_>()(__value);
            const auto __insert_mask = _Load<arch::ISA::AVX512VLF, 128, _IntrinType_>()(
                __mask.__array + __mask.__offset - __index);

            __vector = _Select<arch::ISA::AVX512VLF, 128, _Type_>()(__broadcasted, __vector, __insert_mask);
        }
    }
};

template <> 
struct _Insert<arch::ISA::AVX512VLBW, 128>:
    _Insert<arch::ISA::AVX512VLF, 128> 
{
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (sizeof(_Type_) >= 4)
            return _Insert<arch::ISA::AVX512VLF, 128>()(__vector, __index, __value);

        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
            __vector = __as<_IntrinType_>(_mm_mask_set1_epi16(
                __as<__m128i>(__vector),
                static_cast<uint8>(uint8(1) << __index), 
                memory::pointer_to_integral(__value)));

        else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
            __vector = __as<_IntrinType_>(_mm_mask_set1_epi8(
                __as<__m128i>(__vector),
                static_cast<uint16>(uint16(1) << __index),
                memory::pointer_to_integral(__value)));
    }
};

template <>
struct _Insert<arch::ISA::AVX, 256> {
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_Type_>) {
            const auto __broadcasted = _mm256_broadcast_sd(&__value);

            switch (__index) {
                case 0:
                    __vector = _mm256_blend_pd(__as<__m256d>(__vector), __broadcasted, 1);
                    break;
                case 1:
                    __vector = _mm256_blend_pd(__as<__m256d>(__vector), __broadcasted, 2);
                    break;
                case 2:
                    __vector = _mm256_blend_pd(__as<__m256d>(__vector), __broadcasted, 4);
                    break;
                default:
                    __vector = _mm256_blend_pd(__as<__m256d>(__vector), __broadcasted, 8);
                    break;
            }
        }
        else if constexpr (__is_ps_v<_Type_>) {
            const auto __broadcasted = _mm256_broadcast_ss(&__value);

            switch (__index) {
                case 0:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 1);
                    break;
                case 1:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 2);
                    break;
                case 2:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 4);
                    break;
                case 3:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 8);
                    break;
                case 4:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 0x10);
                    break;
                case 5:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 0x20);
                    break;
                case 6:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 0x40);
                    break;
                default:
                    __vector = _mm256_blend_ps(__as<__m256>(__vector), __broadcasted, 0x80);
                    break;
            }
        }
        else {
            constexpr auto __mask = __simd_make_insert_mask<_IntrinType_, typename IntegerForSizeof<_Type_>::Unsigned>();

            const auto __broadcasted = _Broadcast<arch::ISA::AVX2, 256, _IntrinType_>()(__value);
            const auto __insert_mask = _Load<arch::ISA::AVX2, 256, _IntrinType_>()(__mask.__array + __mask.__offset - __index);

            __vector = _Select<arch::ISA::AVX2, 256, _Type_>()(__broadcasted, __vector, __insert_mask);
        }
	}
};

template <> 
struct _Insert<arch::ISA::AVX2, 256>: 
    _Insert<arch::ISA::AVX, 256> 
{
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
            const auto __broadcasted = _Broadcast<arch::ISA::AVX2, 256, __m256i>()(__value);

            switch (__index) {
                case 0:
                    __vector = __as<_IntrinType_>(_mm256_blend_epi32(
                        __as<__m256i>(__vector), __broadcasted, 0x03));
                    break;
                case 1:
                    __vector = __as<_IntrinType_>(_mm256_blend_epi32(
                        __as<__m256i>(__vector), __broadcasted, 0x0C));
                    break;
                case 2:
                    __vector = __as<_IntrinType_>(_mm256_blend_epi32(
                        __as<__m256i>(__vector), __broadcasted, 0x30));
                    break;
                default:
                    __vector = __as<_IntrinType_>(_mm256_blend_epi32(
                        __as<__m256i>(__vector), __broadcasted, 0xC0));
                    break;
            }
        }
        else {
            return _Insert<arch::ISA::AVX, 256>()(__vector, __index, __value);
        }
    }
};


template <> struct _Insert<arch::ISA::FMA3, 256> : _Insert<arch::ISA::AVX, 256> {};
template <> struct _Insert<arch::ISA::AVX2FMA3, 256> : _Insert<arch::ISA::AVX2, 256> {};

template <> 
struct _Insert<arch::ISA::AVX512VLF, 256>: 
    _Insert<arch::ISA::AVX2, 256> 
{
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm256_mask_set1_epi64(
                __as<__m256i>(__vector),
                static_cast<uint8>(uint8(1) << __index),
                memory::pointer_to_integral(__value)));
        }
        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm256_mask_set1_epi32(
                __as<__m256i>(__vector), 
                static_cast<uint8>(uint8(1) << __index),
                memory::pointer_to_integral(__value)));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm256_mask_broadcastss_ps(
                __as<__m256>(__vector),
                static_cast<uint8>(uint8(1) << __index),
                _mm_set_ss(__value)));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm256_mask_broadcastsd_pd(
                __as<__m256d>(__vector),
                static_cast<uint8>(uint8(1) << __index),
                _mm_set_sd(__value)));
        }
        else {
            const auto __mask = __simd_make_insert_mask<_IntrinType_, typename IntegerForSizeof<_Type_>::Unsigned>();

            const auto __broadcasted = _Broadcast<arch::ISA::AVX512VLF, 256, _IntrinType_>()(__value);
            const auto __insert_mask = _Load<arch::ISA::AVX512VLF, 256, _IntrinType_>()(
                __mask.__array + __mask.__offset - __index);

            __vector = _Select<arch::ISA::AVX512VLF, 256, _Type_>()(__broadcasted, __vector, __insert_mask);
        }
    }
};

template <> 
struct _Insert<arch::ISA::AVX512VLBW, 256>:
    _Insert<arch::ISA::AVX512VLF, 256> 
{
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (sizeof(_Type_) >= 4)
            return _Insert<arch::ISA::AVX512VLF, 256>()(__vector, __index, __value);

        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
            __vector = __as<_IntrinType_>(_mm256_mask_set1_epi16(
                __as<__m256i>(__vector),
                static_cast<uint16>(uint16(1) << __index), 
                memory::pointer_to_integral(__value)));

        else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
            __vector = __as<_IntrinType_>(_mm256_mask_set1_epi8(
                __as<__m256i>(__vector),
                static_cast<uint32>(uint32(1) << __index),
                memory::pointer_to_integral(__value)));
    }
};

template <>
struct _Insert<arch::ISA::AVX512F, 512> {
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_&   __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm512_mask_set1_epi64(
                __as<__m512i>(__vector), 
                static_cast<uint8>(uint8(1) << __index),
                memory::pointer_to_integral(__value)));
        }
        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm512_mask_set1_epi32(
                __as<__m512i>(__vector), 
                static_cast<uint16>(uint16(1) << __index),
                memory::pointer_to_integral(__value)));
        }
        else if constexpr (__is_ps_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm512_mask_broadcastss_ps(
                __as<__m512>(__vector), 
                static_cast<uint16>(uint16(1) << __index),
                _mm_set_ss(__value)));
        }
        else if constexpr (__is_pd_v<_Type_>) {
            __vector = __as<_IntrinType_>(_mm512_mask_broadcastsd_pd(
                __as<__m512d>(__vector),
                static_cast<uint8>(uint8(1) << __index),
                _mm_set_sd(__value)));
        }
        else {
            const auto __mask = __simd_make_insert_mask<_IntrinType_, typename IntegerForSizeof<_Type_>::Unsigned>();

            const auto __broadcasted = _Broadcast<arch::ISA::AVX512F, 512, _IntrinType_>()(__value);
            const auto __insert_mask = _Load<arch::ISA::AVX512F, 512, _IntrinType_>()(__mask.__array + __mask.__offset - __index);

            __vector = _Select<arch::ISA::AVX512F, 512, _Type_>()(__broadcasted, __vector, __insert_mask);
        }
	}
};

template <>  
struct _Insert<arch::ISA::AVX512BW, 512>: 
    _Insert<arch::ISA::AVX512F, 512> 
{
    template <
        class _IntrinType_,
        class _Type_>
    raze_static_operator raze_always_inline void operator()(
        _IntrinType_& __vector,
        uint8			__index,
        _Type_	__value) raze_const_operator noexcept
    {
        if constexpr (sizeof(_Type_) >= 4)
            return _Insert<arch::ISA::AVX512F, 512>()(__vector, __index, __value);

        else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
            __vector = __as<_IntrinType_>(_mm512_mask_set1_epi16(
                __as<__m512i>(__vector), 
                static_cast<uint32>(uint32(1) << __index), 
                memory::pointer_to_integral(__value)));

        else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
            __vector = __as<_IntrinType_>(_mm512_mask_set1_epi8(
                __as<__m512i>(__vector),
                static_cast<uint64>(uint64(1) << __index),
                memory::pointer_to_integral(__value)));
    }
};


template <> struct _Insert<arch::ISA::AVX512DQ, 512>: _Insert<arch::ISA::AVX512F, 512> {};
template <> struct _Insert<arch::ISA::AVX512BWDQ, 512>: _Insert<arch::ISA::AVX512BW, 512> {};
template <> struct _Insert<arch::ISA::AVX512VBMI, 512>: _Insert<arch::ISA::AVX512BW, 512> {};
template <> struct _Insert<arch::ISA::AVX512VBMI2, 512>: _Insert<arch::ISA::AVX512VBMI, 512> {};
template <> struct _Insert<arch::ISA::AVX512VBMIDQ, 512>: _Insert<arch::ISA::AVX512BWDQ, 512> {};
template <> struct _Insert<arch::ISA::AVX512VBMI2DQ, 512>: _Insert<arch::ISA::AVX512VBMIDQ, 512> {};

template <> struct _Insert<arch::ISA::AVX512VLDQ, 256>: _Insert<arch::ISA::AVX512VLF, 256> {};
template <> struct _Insert<arch::ISA::AVX512VLBWDQ, 256>: _Insert<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Insert<arch::ISA::AVX512VBMIVL, 256>: _Insert<arch::ISA::AVX512VLBW, 256> {};
template <> struct _Insert<arch::ISA::AVX512VBMI2VL, 256>: _Insert<arch::ISA::AVX512VBMIVL, 256> {};
template <> struct _Insert<arch::ISA::AVX512VBMIVLDQ, 256>: _Insert<arch::ISA::AVX512VLBWDQ, 256> {};
template <> struct _Insert<arch::ISA::AVX512VBMI2VLDQ, 256>: _Insert<arch::ISA::AVX512VBMIVLDQ, 256> {};

template <> struct _Insert<arch::ISA::AVX512VLDQ, 128>: _Insert<arch::ISA::AVX512VLF, 128> {};
template <> struct _Insert<arch::ISA::AVX512VLBWDQ, 128>: _Insert<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Insert<arch::ISA::AVX512VBMIVL, 128>: _Insert<arch::ISA::AVX512VLBW, 128> {};
template <> struct _Insert<arch::ISA::AVX512VBMI2VL, 128>: _Insert<arch::ISA::AVX512VBMIVL, 128> {};
template <> struct _Insert<arch::ISA::AVX512VBMIVLDQ, 128>: _Insert<arch::ISA::AVX512VLBWDQ, 128> {};
template <> struct _Insert<arch::ISA::AVX512VBMI2VLDQ, 128>: _Insert<arch::ISA::AVX512VBMIVLDQ, 128> {};

__RAZE_VX_NAMESPACE_END
