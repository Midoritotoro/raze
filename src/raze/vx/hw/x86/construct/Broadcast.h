#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <raze/math/BitCast.h>
#include <src/raze/vx/hw/x86/memory/Load.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_>
concept has_value_type = requires { typename _Type_::value_type; };


template <class _Type_, class = void>
struct __unwrap_int {
	using type = _Type_;
};

template <class _Type_>
struct __unwrap_int<_Type_, std::void_t<typename _Type_::value_type>> {
	using type = typename _Type_::value_type;
};

template <arch::ISA	_ISA_, intrin_or_arithmetic_type _Tp_>
struct _Broadcast {
	template <class _WrappedType_>
	raze_nodiscard raze_static_operator raze_no_stack_protector raze_always_inline
		_Tp_ operator()(_WrappedType_ __value) raze_const_operator noexcept
			requires(arithmetic_type<_WrappedType_> || has_value_type<_WrappedType_>)
	{
		using _Type_ = typename __unwrap_int<_WrappedType_>::type;

		if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm_broadcastq_epi64(_mm_cvtsi64_si128(math::pointer_to_integral(__value))));
				else return __as<_Tp_>(_mm_set1_epi64x(math::pointer_to_integral(__value)));
			}
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm_broadcastd_epi32(_mm_cvtsi32_si128(math::pointer_to_integral(__value))));
				else return __as<_Tp_>(_mm_set1_epi32(math::pointer_to_integral(__value)));
			}
			else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm_broadcastw_epi16(_mm_cvtsi32_si128(__value)));
				else return __as<_Tp_>(_mm_set1_epi16(__value));
			}
			else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm_broadcastb_epi8(_mm_cvtsi32_si128(__value)));
				else return __as<_Tp_>(_mm_set1_epi8(__value));
			}
			else if constexpr (__is_ps_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm_broadcastss_ps(_mm_set_ss(__value)));
				else return __as<_Tp_>(_mm_set1_ps(__value));
			}
			else if constexpr (__is_pd_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm_broadcastsd_pd(_mm_set_sd(__value)));
				else return __as<_Tp_>(_mm_set1_pd(__value));
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm256_broadcastq_epi64(_mm_cvtsi64_si128(math::pointer_to_integral(__value))));
				else return __as<_Tp_>(_mm256_set1_epi64x(math::pointer_to_integral(__value)));
			}
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm256_broadcastd_epi32(_mm_cvtsi32_si128(math::pointer_to_integral(__value))));
				else return __as<_Tp_>(_mm256_set1_epi32(math::pointer_to_integral(__value)));
			}
			else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm256_broadcastw_epi16(_mm_cvtsi32_si128(__value)));
				else return __as<_Tp_>(_mm256_set1_epi16(__value));
			}
			else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm256_broadcastb_epi8(_mm_cvtsi32_si128(__value)));
				else return __as<_Tp_>(_mm256_set1_epi8(__value));
			}
			else if constexpr (__is_ps_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm256_broadcastss_ps(_mm_set_ss(__value)));
				else return __as<_Tp_>(_mm256_set1_ps(__value));
			}
			else if constexpr (__is_pd_v<_Type_>) {
				if constexpr (__has_avx2_support_v<_ISA_>) return __as<_Tp_>(_mm256_broadcastsd_pd(_mm_set_sd(__value)));
				else return __as<_Tp_>(_mm256_set1_pd(__value));
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) return __as<_Tp_>(_mm512_set1_epi64(math::pointer_to_integral(__value))); 
			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) return __as<_Tp_>(_mm512_set1_epi32(math::pointer_to_integral(__value)));
			else if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
				if constexpr (__has_avx512bw_support_v<_ISA_>) return __as<_Tp_>(_mm512_set1_epi16(__value));
				else {
					i32 __x = __value;
					__x |= __x << 16;
					return __as<_Tp_>(_mm512_set1_epi32(__x));
				}
			}
			else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
				if constexpr (__has_avx512bw_support_v<_ISA_>) return __as<_Tp_>(_mm512_set1_epi8(__value));
				else return _mm512_set1_epi32(static_cast<u8>(__value) * 0x01010101u);
			}
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_set1_ps(__value));
			else if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_set1_pd(__value));
		}
		else {
			return __value;
		}
	}
};

__RAZE_VX_NAMESPACE_END
