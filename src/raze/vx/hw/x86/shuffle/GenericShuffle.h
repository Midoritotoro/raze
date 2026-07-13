#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/x86/compare/Greater.h>
#include <src/raze/vx/hw/configurable/merge/Select.h>
#include <src/raze/vx/hw/configurable/shape/Split.h>
#include <src/raze/vx/hw/configurable/shape/SplitBy.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Tp_>
struct _Fallback_result {
	static constexpr auto __fallback = true;

	_Fallback_result(_Tp_ __x) noexcept : _data(__x) {}

	_Fallback_result(_Fallback_result&&) noexcept = default;
	_Fallback_result(const _Fallback_result&) noexcept = default;

	_Fallback_result& operator=(const _Fallback_result&) noexcept = default;
	_Fallback_result& operator=(_Fallback_result&&) noexcept = default;

	raze_always_inline operator _Tp_() const noexcept {
		return _data;
	}

	_Tp_ _data;
};


template <intrin_type _Intrin_, class _IdxType_>
struct _Rotate_indices {
	using index_type = _IdxType_;
	_Rotate_indices(_Intrin_ __x) noexcept : _data(__x) {}

	_Rotate_indices(const _Rotate_indices&) noexcept = default;
	_Rotate_indices(_Rotate_indices&&) noexcept = default;

	_Rotate_indices& operator=(const _Rotate_indices&) noexcept = default;
	_Rotate_indices& operator=(_Rotate_indices&&) noexcept = default;

	raze_always_inline operator _Intrin_() const noexcept {
		return _data;
	}

	raze_always_inline _Intrin_ data() const noexcept {
		return _data;
	}

	_Intrin_ _data;
};

template <arch::ISA _ISA_, class _Type_,
	intrin_type _Intrin_, sizetype ...	_Indices_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Fallback_result<_Intrin_> __shuffle_fallback(
	_Intrin_ __vector, std::integer_sequence<sizetype, _Indices_...>) noexcept
{
	constexpr auto __length = sizeof(_Intrin_) / sizeof(_Type_);
	using _IndexType = typename IntegerForSizeof<_Type_>::Unsigned;

	alignas(sizeof(_Intrin_)) static constexpr _IndexType __indices_array[] = { _Indices_... };

	alignas(sizeof(_Intrin_)) _Type_ __src[__length];
	alignas(sizeof(_Intrin_)) _Type_ __dst[__length];

	_Store<_ISA_>()(__src, __vector, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__dst[__i] = __src[__indices_array[__i]];

	return _Load<_ISA_, _Intrin_>()(__dst, __aligned_policy{});
}

template <arch::ISA _ISA_, class _Type_,
	intrin_type _Intrin_, intrin_type _Index_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Fallback_result<_Intrin_> __shuffle_fallback(
	_Intrin_ __vector, _Index_ __idx) noexcept
{
	constexpr auto __length = sizeof(_Intrin_) / sizeof(_Type_);
	using _IndexType = typename IntegerForSizeof<_Type_>::Unsigned;

	alignas(sizeof(_Intrin_)) _Type_ __src[__length];
	alignas(sizeof(_Intrin_)) _Type_ __dst[__length];
	alignas(sizeof(_Intrin_)) _IndexType __idxs[__length];

	_Store<_ISA_>()(__src, __vector, __aligned_policy{});
	_Store<_ISA_>()(__idxs, __idx, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__dst[__i] = __src[__idxs[__i]];

	return _Load<_ISA_, _Intrin_>()(__dst, __aligned_policy{});
}

template <class _Pattern_>
raze_no_stack_protector raze_always_inline _Fallback_result<pattern_vector_t<_Pattern_>> 
__generic_shuffle_scalar_fallback(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept 
{
	using _Simd_ = pattern_vector_t<_Pattern_>;

	alignas(64) typename _Simd_::value_type __dst[_Simd_::size()];
	alignas(64) typename _Simd_::value_type __src[_Simd_::size()];

	vx::__store[vx::aligned](__src, __x);

	[&] <sizetype ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
		([&](auto __i) raze_always_inline_lambda{
			__dst[__i] = __src[__p[__i]];
		}(std::integral_constant<sizetype, __I>{}), ...);
	}(std::make_integer_sequence<sizetype, _Simd_::size()>{});

	return vx::__load<_Simd_>[vx::aligned](__dst);
}

template <simd_type _Simd_, index_simd_type _Index_>
raze_no_stack_protector raze_always_inline _Fallback_result<_Simd_> 
__generic_shuffle_scalar_fallback(const _Simd_& __x, const _Index_& __idx) noexcept 
{
	alignas(64) typename _Simd_::value_type __dst[_Simd_::size()];
	alignas(64) typename _Simd_::value_type __src[_Simd_::size()];
	alignas(64) typename _Index_::value_type __idxs[_Index_::size()];

	vx::__store[vx::aligned](__src, __x);
	vx::__store[vx::aligned](__idxs, __idx);

	if constexpr (_Simd_::size() <= 32) {
		[&] <sizetype ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
			([&](auto __i) raze_always_inline_lambda{
				__dst[__i] = __src[__idxs[__i]];
			}(std::integral_constant<sizetype, __I>{}), ...);
		}(std::make_integer_sequence<sizetype, _Simd_::size()>{});
	}
	else {
		for (auto __i = 0; __i < _Simd_::size(); ++__i)
			__dst[__i] = __src[__idxs[__i]];
	}

	return vx::__load<_Simd_>[vx::aligned](__dst);
}

template <arch::ISA _ISA_, intrin_type _Intrin_>
raze_no_stack_protector raze_always_inline _Intrin_ __byte_shuffle_mask_avx2(_Intrin_ __x) noexcept {
	static_assert(__has_avx2_support_v<_ISA_>);

	// Multiplication by 0x0202 (= {0x02, 0x02} as a pair of bytes) creates pairs {2*i, 2*i}
	// Adding 0x0100 increments the high byte of each pair (resulting in {2*i, 2*i+1}).
	return __as<_Intrin_>(_mm256_add_epi16(_mm256_mullo_epi16(__as<__m256i>(__x), 
		_Broadcast<_ISA_, __m256i>()(std::integral_constant<i16, 0x202>{})),
		_Broadcast<_ISA_, __m256i>()(std::integral_constant<i16, 0x0100>{})));
}

template <arch::ISA _ISA_, intrin_type _Intrin_, intrin_type _Index_>
raze_no_stack_protector raze_always_inline _Intrin_ __shuffle_i8x64_avx512(_Intrin_ __x, _Index_ __idx) noexcept {
	static_assert(sizeof(_Intrin_) == sizeof(__m512i));

	if constexpr (__has_avx512vbmi_support_v<_ISA_>) {
		return __as<_Intrin_>(_mm512_permutexvar_epi8(__as<__m512i>(__idx), __as<__m512i>(__x)));
	}
	else if constexpr (__has_avx512bw_support_v<_ISA_>) {
		auto __src_part = _mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0);
		auto __result = _mm512_shuffle_epi8(__src_part, __as<__m512i>(__idx));

		__src_part = _mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0x55);
		auto __candidate = _mm512_shuffle_epi8(__src_part, __as<__m512i>(__idx));
		auto __threshold = _Broadcast<_ISA_, __m512i>()(std::integral_constant<i8, 0x0F>{});
		__result = _mm512_mask_mov_epi8(__result, _mm512_cmpgt_epi8_mask(__as<__m512i>(__idx), __threshold), __candidate);

		__src_part = _mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0xAA);
		__candidate = _mm512_shuffle_epi8(__src_part, __as<__m512i>(__idx));
		__threshold = _Broadcast<_ISA_, __m512i>()(std::integral_constant<i8, 0x1F>{});
		__result = _mm512_mask_mov_epi8(__result, _mm512_cmpgt_epi8_mask(__as<__m512i>(__idx), __threshold), __candidate);

		__src_part = _mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0xFF);
		__candidate = _mm512_shuffle_epi8(__src_part, __as<__m512i>(__idx));
		__threshold = _Broadcast<_ISA_, __m512i>()(std::integral_constant<i8, 0x2F>{});
		__result = _mm512_mask_mov_epi8(__result, _mm512_cmpgt_epi8_mask(__as<__m512i>(__idx), __threshold), __candidate);

		return __result;
	}
	else {
		const auto __idx_high = _mm512_extracti64x4_epi64(__as<__m512i>(__idx), 1);
		auto __src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0));

		auto __result_low = _mm256_shuffle_epi8(__src_part, __as<__m256i>(__idx));
		auto __result_high = _mm256_shuffle_epi8(__src_part, __idx_high);

		__src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0x05));
		auto __candidate_low = _mm256_shuffle_epi8(__src_part, __as<__m256i>(__idx));
		auto __candidate_high = _mm256_shuffle_epi8(__src_part, __idx_high);

		auto __threshold = _Broadcast<_ISA_, __m256i>()(std::integral_constant<i8, 0x0F>{});
		__result_low = _mm256_blendv_epi8(__result_low, __candidate_low, _mm256_cmpgt_epi8(__as<__m256i>(__idx), __threshold));
		__result_high = _mm256_blendv_epi8(__result_high, __candidate_high, _mm256_cmpgt_epi8(__idx_high, __threshold));

		__src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0xA));

		__candidate_low = _mm256_shuffle_epi8(__src_part, __as<__m256i>(__idx));
		__candidate_high = _mm256_shuffle_epi8(__src_part, __idx_high);

		__threshold = _Broadcast<_ISA_, __m256i>()(std::integral_constant<i8, 0x1F>{});
		__result_low = _mm256_blendv_epi8(__result_low, __candidate_low, _mm256_cmpgt_epi8(__as<__m256i>(__idx), __threshold));
		__result_high = _mm256_blendv_epi8(__result_high, __candidate_high, _mm256_cmpgt_epi8(__idx_high, __threshold));

		__src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0x1F));

		__candidate_low = _mm256_shuffle_epi8(__src_part, __as<__m256i>(__idx));
		__candidate_high = _mm256_shuffle_epi8(__src_part, __idx_high);

		__threshold = _Broadcast<_ISA_, __m256i>()(std::integral_constant<i8, 0x2F>{});
		__result_low = _mm256_blendv_epi8(__result_low, __candidate_low, _mm256_cmpgt_epi8(__as<__m256i>(__idx), __threshold));
		__result_high = _mm256_blendv_epi8(__result_high, __candidate_high, _mm256_cmpgt_epi8(__idx_high, __threshold));

		return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__result_low), __result_high, 1));
	}
}

template <arch::ISA _ISA_, intrin_type _Intrin_, intrin_type _Index_>
raze_no_stack_protector raze_always_inline _Intrin_ __shuffle_i16x32_avx512(_Intrin_ __x, _Index_ __idx) noexcept {
	static_assert(sizeof(_Intrin_) == sizeof(__m512i));

	if constexpr (__has_avx512bw_support_v<_ISA_>) {
		return __as<_Intrin_>(_mm512_permutexvar_epi16(__as<__m512i>(__idx), __as<__m512i>(__x)));
	}
	else {
		// shuffle_i64x2 layouts:
		// 0x00 : {L0,L0,L0,L0}
		// 0x05 : {L1,L1,L1,L1}
		// 0x0A : {L2,L2,L2,L2}
		// 0x0F : {L3,L3,L3,L3}

		const auto __idx_high = _mm512_extracti64x4_epi64(__as<__m512i>(__idx), 1);

		const auto __byte_shuffle_mask_low = __byte_shuffle_mask_avx2<_ISA_>(__as<__m256i>(__idx));
		const auto __byte_shuffle_mask_high = __byte_shuffle_mask_avx2<_ISA_>(__idx_high);

		auto __src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0));

		auto __result_low = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_low);
		auto __result_high = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_high);

		__src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0x05));
		auto __candidate_low = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_low);
		auto __candidate_high = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_high);

		auto __threshold = _Broadcast<_ISA_, __m256i>()(std::integral_constant<i16, 0x07>{});
		__result_low = _mm256_blendv_epi8(__result_low, __candidate_low, _mm256_cmpgt_epi16(__as<__m256i>(__idx), __threshold));
		__result_high = _mm256_blendv_epi8(__result_high, __candidate_high, _mm256_cmpgt_epi16(__idx_high, __threshold));

		__src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0xA));

		__candidate_low = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_low);
		__candidate_high = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_high);

		__threshold = _Broadcast<_ISA_, __m256i>()(std::integral_constant<i16, 0x0F>{});
		__result_low = _mm256_blendv_epi8(__result_low, __candidate_low, _mm256_cmpgt_epi16(__as<__m256i>(__idx), __threshold));
		__result_high = _mm256_blendv_epi8(__result_high, __candidate_high, _mm256_cmpgt_epi16(__idx_high, __threshold));

		__src_part = __as<__m256i>(_mm512_shuffle_i64x2(__as<__m512i>(__x), __as<__m512i>(__x), 0xF));

		__candidate_low = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_low);
		__candidate_high = _mm256_shuffle_epi8(__src_part, __byte_shuffle_mask_high);

		__threshold = _Broadcast<_ISA_, __m256i>()(std::integral_constant<i16, 0x17>{});
		__result_low = _mm256_blendv_epi8(__result_low, __candidate_low, _mm256_cmpgt_epi16(__as<__m256i>(__idx), __threshold));
		__result_high = _mm256_blendv_epi8(__result_high, __candidate_high, _mm256_cmpgt_epi16(__idx_high, __threshold));

		return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__result_low), __result_high, 1));
	}
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_, class _Pattern_>
raze_no_stack_protector raze_always_inline auto __generic_shuffle_native(_Intrin_ __x, _Pattern_ __p) noexcept {
	static constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
	static constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
	static constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
	static constexpr auto __avx512f = __has_avx512f_support_v<_ISA_>;
	static constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
	static constexpr auto __avx512vbmi = __has_avx512vbmi_support_v<_ISA_>;

	if constexpr (sizeof(_Intrin_) == 16) {
		if constexpr (sizeof(_Type_) == 8) {
			constexpr auto __mask = __shufpd_to_pshufd_mask(__p);
			return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), __mask));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			constexpr auto __mask = __to_pshufd_mask(__p);
			return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), __mask));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			constexpr auto __low_shuf = __to_pshufd_mask(__p);
			constexpr auto __high_shuf = (__p[4] & 0x03) | ((__p[5] & 0x03) << 2) | ((__p[6] & 0x03) << 4) | ((__p[7] & 0x03) << 6);

			if constexpr (__can_widen_shuffle(__p)) {
				return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), __to_pshufd_mask(__p.widen())));
			}
			else if constexpr (!__across_halfs(__p)) {
				auto __shuffled = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_shufflehi_epi16(__shuffled, __high_shuf));
			}
			else if constexpr (__ssse3) {
				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __p.template expand<u16, u8>().template as_native<__m128i>()));
			}
			else if constexpr (__is_dup_low(__p)) {
				const auto __low = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_unpacklo_epi64(__low, __low));
			}
			else if constexpr (__is_dup_high(__p)) {
				const auto __high = _mm_shufflehi_epi16(__as<__m128i>(__x), __high_shuf);
				return __as<_Intrin_>(_mm_unpackhi_epi64(__high, __high));
			}
			else if constexpr (__is_low_half(__p)) {
				const auto __low1 = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __low2 = _mm_shufflelo_epi16(__as<__m128i>(__x), __high_shuf);
				return __as<_Intrin_>(_mm_unpacklo_epi64(__low1, __low2));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __high1 = _mm_shufflehi_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __high2 = _mm_shufflehi_epi16(__as<__m128i>(__x), __high_shuf);
				return __as<_Intrin_>(_mm_unpacklo_epi64(__high1, __high2));
			}
			else {
				const auto __low1 = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __low2 = _mm_shufflelo_epi16(__as<__m128i>(__x), __high_shuf);
				const auto __high1 = _mm_shufflehi_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __high2 = _mm_shufflehi_epi16(__as<__m128i>(__x), __high_shuf);

				const auto __low = _mm_unpacklo_epi64(__low1, __low2);
				const auto __high = _mm_unpackhi_epi64(__high1, __high2);

				return _Select<_ISA_, _Type_>()(__low, __high, _To_mask<_ISA_, _Type_>()(__p < std::integral_constant<sizetype, 4>{}));
			}
		}
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (__can_widen_shuffle(__p) && !__across_halfs(__p)) {
				constexpr auto __widen = __p.widen();

				constexpr auto __low_shuf = __to_pshufd_mask(__widen);
				constexpr auto __high_shuf = (__widen[4] & 0x03) | ((__widen[5] & 0x03) << 2)
					| ((__widen[6] & 0x03) << 4) | ((__widen[7] & 0x03) << 6);

				auto __shuffled = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_shufflehi_epi16(__shuffled, __high_shuf));
			}
			else if constexpr (__ssse3) {
				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __p.template as_native<__m128i>()));
			}
			else return __shuffle_fallback<_ISA_, _Type_>(__x, __p.get());
		}
		else return __shuffle_fallback<_ISA_, _Type_>(__x, __p.get());
	}
	else if constexpr (sizeof(_Intrin_) == 32) {
		if constexpr (sizeof(_Type_) == 8) {
			constexpr auto __mask = ((__p[0] % 2)) | ((__p[1] % 2) << 1) | ((__p[2] % 2) << 2) | ((__p[3] % 2) << 3);

			if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_permute_pd(__as<__m256d>(__x), __mask));
			}
			else if constexpr (__avx2) return __as<_Intrin_>(_mm256_permute4x64_epi64(__as<__m256i>(__x), __to_pshufd_mask(__p)));
			else if constexpr (__is_low_half(__p)) {
				const auto __broadcasted_low_lane = _mm256_permute2f128_pd(__as<__m256d>(__x), __as<__m256d>(__x), 0);
				return __as<_Intrin_>(_mm256_permute_pd(__broadcasted_low_lane, __mask));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __broadcasted_high_lane = _mm256_permute2f128_pd(__as<__m256d>(__x), __as<__m256d>(__x), 0x11);
				return __as<_Intrin_>(_mm256_permute_pd(__broadcasted_high_lane, __mask));
			}
			else {
				constexpr auto __blend_mask = (__p / std::integral_constant<sizetype, 2>{}) != _Shuffle_pattern<pattern_vector_t<_Pattern_>, 0, 0, 1, 1>{};
				const auto __swapped_lanes = _mm256_permute2f128_pd(__as<__m256d>(__x), __as<__m256d>(__x), 0x01);

				const auto __first = _mm256_permute_pd(__as<__m256d>(__x), __mask);
				const auto __second = _mm256_permute_pd(__swapped_lanes, __mask);
				
				return __as<_Intrin_>(_mm256_blend_pd(__first, __second, __blend_mask));
			}
		}
		else if constexpr (sizeof(_Type_) == 4) {
			if constexpr (!__across_halfs(__p))
				return __as<_Intrin_>(_mm256_shuffle_ps(__as<__m256>(__x), __as<__m256>(__x), __to_pshufd_mask(__p)));
			else if constexpr (__can_widen_shuffle(__p) && __avx2)
				return __as<_Intrin_>(_mm256_permute4x64_epi64(__as<__m256i>(__x), __to_pshufd_mask(__p.widen())));
			else if constexpr (__avx2)
				return __as<_Intrin_>(_mm256_permutevar8x32_epi32(__as<__m256i>(__x), __p.template as_native<__m256i>()));
			else if constexpr (__is_dup_low(__p)) {
				auto __split = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0);
				
				if constexpr (!__is_dup_low_identity(__p))
					__split = _mm256_permute_ps(__split, __to_pshufd_mask(__p));

				return __as<_Intrin_>(__split);
			}
			else if constexpr (__is_dup_high(__p)) {
				constexpr auto __mask = (((__p[4] - 4) & 0x03) | (((__p[5] - 4) & 0x03) << 2)
					| (((__p[6] - 4) & 0x03) << 4) | (((__p[7] - 4) & 0x03) << 6));

				auto __split = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0x11);

				if constexpr (!__is_dup_low_identity(__p))
					__split = _mm256_permute_ps(__split, __mask);

				return __as<_Intrin_>(__split);
			}
			else if constexpr (__is_low_half(__p)) {
				const auto __broadcasted_low_lane = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0);
				return __as<_Intrin_>(_mm256_permutevar_ps(__broadcasted_low_lane, (__p % std::integral_constant<sizetype, 4>{}).template as_native<__m256i>()));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __broadcasted_low_lane = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0x11);
				return __as<_Intrin_>(_mm256_permutevar_ps(__broadcasted_low_lane, (__p % std::integral_constant<sizetype, 4>{}).template as_native<__m256i>()));
			}
			else {
				constexpr auto __low_mask = (__p % std::integral_constant<sizetype, 4>{});
				constexpr auto __blend_mask = (__p / std::integral_constant<sizetype, 4>{}) != _Shuffle_pattern<pattern_vector_t<_Pattern_>, 0, 0, 0, 0, 1, 1, 1, 1>{};
				const auto __swapped_lanes = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0x01);

				const auto __native_mask = __p.template as_native<__m256i>();

				const auto __first = _mm256_permutevar_ps(__as<__m256>(__x), __native_mask);
				const auto __second = _mm256_permutevar_ps(__swapped_lanes, __native_mask);

				return __as<_Intrin_>(_mm256_blend_ps(__first, __second, __blend_mask));
			}
		}
		else if constexpr (sizeof(_Type_) == 2 && __avx2) {
			constexpr auto __expanded = __p.template expand<u16, u8>();
			constexpr auto __mask = __expanded % std::integral_constant<sizetype, 16>{};

			if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_shuffle_epi8(__as<__m256i>(__x), __p.expand<u16, u8>().template as_native<__m256i>()));
			}
			else if constexpr (__avx512bw && __avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi16(__p.template as_native<__m256i>(), __as<__m256i>(__x)));
			}
			else if constexpr (__is_low_half(__p)) { 
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else if constexpr (__is_high_half(__p)) { 
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x11);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else {
				const auto __swapped = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x01);

				const auto __shuffled1 = _mm256_shuffle_epi8(__as<__m256i>(__x), __expanded.crossing_lanes().template as_native<__m256i>());
				const auto __shuffled2 = _mm256_shuffle_epi8(__swapped, __expanded.non_crossing_lanes().template as_native<__m256i>());

				return __as<_Intrin_>(_mm256_or_si256(__shuffled1, __shuffled2));
			}
		}
		else if constexpr (sizeof(_Type_) == 1 && __avx2) {
			constexpr auto __mask = (__p % std::integral_constant<sizetype, 16>{});

			if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_shuffle_epi8(__as<__m256i>(__x), __p.template as_native<__m256i>()));
			}
			else if constexpr (__avx512vbmi && __avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi8(__p.template as_native<__m256i>(), __as<__m256i>(__x)));
			}
			else if constexpr (__is_low_half(__p)) {
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x11);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else {
				const auto __shuffled1 = _mm256_shuffle_epi8(__as<__m256i>(__x), __p.crossing_lanes().template as_native<__m256i>());
				const auto __shuffled2 = _mm256_shuffle_epi8(_mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x01),
					__p.non_crossing_lanes().template as_native<__m256i>());

				return __as<_Intrin_>(_mm256_or_si256(__shuffled1, __shuffled2));
			}
		}
		else return __shuffle_fallback<_ISA_, _Type_>(__x, __p.get());
	}
	else if constexpr (sizeof(_Intrin_) == 64) {
		if constexpr (sizeof(_Type_) == 8) {
			if constexpr (!__across_quads(__p)) {
				constexpr auto __mask = ((__p[0] & 1) << 0) | ((__p[1] & 1) << 1) | ((__p[2] & 1) << 2) | ((__p[3] & 1) << 3) |
					((__p[4] & 1) << 4) | ((__p[5] & 1) << 5) | ((__p[6] & 1) << 6) | ((__p[7] & 1) << 7);

				return __as<_Intrin_>(_mm512_permute_pd(__as<__m512d>(__x), __mask));
			}
			return __as<_Intrin_>(_mm512_permutexvar_epi64(__p.template as_native<__m512i>(), __as<__m512i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			if constexpr (!__across_quads(__p)) {
				constexpr auto __q0 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 0>{}));
				constexpr auto __q1 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 4>{}));
				constexpr auto __q2 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 8>{}));
				constexpr auto __q3 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 12>{}));

				if constexpr (__q0 == __q1 && __q1 == __q2 && __q2 == __q3) return __as<_Intrin_>(_mm512_permute_ps(__as<__m512>(__x), __q0));
				else return __as<_Intrin_>(_mm512_permutevar_ps(__as<__m512>(__x), __p.template as_native<__m512i>()));
			}
			else return __as<_Intrin_>(_mm512_permutevar_epi32(__p.template as_native<__m512i>(), __as<__m512i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			if constexpr (__avx512bw) return __as<_Intrin_>(_mm512_permutexvar_epi16(__p.template as_native<__m512i>(), __as<__m512i>(__x)));
			else if constexpr (!__across_quads(__p)) {
				const auto __shuffle = __p.template expand<u16, u8>().template as_native<__m512i>();

				const auto __low = _mm256_shuffle_epi8(__as<__m256i>(__x), __as<__m256i>(__shuffle));
				const auto __high = _mm256_shuffle_epi8(_mm512_extracti64x4_epi64(__as<__m512i>(__x), 1), _mm512_extracti64x4_epi64(__as<__m512i>(__shuffle), 1));

				return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
			}
			else return __shuffle_i8x64_avx512<_ISA_>(__x, __p.template expand<u16, u8>().template as_native<__m512i>());
		}
		else if constexpr (sizeof(_Type_) == 1) {
			const auto __shuffle = __p.template as_native<__m512i>();

			if constexpr (!__across_quads(__p) && (!__avx512vbmi || __avx512bw)) {
				if constexpr (__avx512bw) return __as<_Intrin_>(_mm512_shuffle_epi8(__as<__m512i>(__x), __shuffle));
				else {
					const auto __low = _mm256_shuffle_epi8(__as<__m256i>(__x), __as<__m256i>(__shuffle));
					const auto __high = _mm256_shuffle_epi8(_mm512_extracti64x4_epi64(__as<__m512i>(__x), 1), _mm512_extracti64x4_epi64(__as<__m512i>(__shuffle), 1));

					return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
				}
			}
			else return __shuffle_i8x64_avx512<_ISA_>(__x, __p.template as_native<__m512i>());
		}
		else return __shuffle_fallback<_ISA_, _Type_>(__x, __p.get());
	}
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_, intrin_type _Index_>
raze_no_stack_protector raze_always_inline auto __generic_shuffle_native(_Intrin_ __x, _Index_ __idx) noexcept {
	static constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
	static constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
	static constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
	static constexpr auto __avx512f = __has_avx512f_support_v<_ISA_>;
	static constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
	static constexpr auto __avx512vbmi = __has_avx512vbmi_support_v<_ISA_>;

	if constexpr (sizeof(_Intrin_) == 16) {
		if constexpr (sizeof(_Type_) == 8) {
			if constexpr (__avx512vl) {
				return __as<_Intrin_>(_mm_permutex2var_epi64(__as<__m128i>(__x), __as<__m128i>(__idx), __as<__m128i>(__x)));
			}
			else if constexpr (__ssse3) {
				const auto __dupe_mask = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8);
				const auto __add_seq = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);

				const auto __byte_offsets = _mm_slli_epi64(__as<__m128i>(__idx), 3);
				const auto __duplicated = _mm_shuffle_epi8(__byte_offsets, __dupe_mask);
				const auto __mask = _mm_add_epi8(__duplicated, __add_seq);

				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __mask));
			}
			else {
				alignas(16) _Type_ __t[2];
				_Store<_ISA_>()(__t, __x, __aligned_policy{});

				const auto* __i = reinterpret_cast<const unsigned long long*>(&__idx);
				
				if constexpr (std::is_floating_point_v<_Type_>) return _Fallback_result { __as<_Intrin_>(_mm_set_pd(__t[__i[1]], __t[__i[0]])) };
				else return _Fallback_result{ __as<_Intrin_>(_mm_set_epi64x(__t[__i[1]], __t[__i[0]])) };
			}
		}
		else if constexpr (sizeof(_Type_) == 4) {
			if constexpr (__avx512vl) {
				return __as<_Intrin_>(_mm_permutex2var_epi32(__as<__m128i>(__x), __as<__m128i>(__idx), __as<__m128i>(__x)));
			}
			else if constexpr (__ssse3) {
				const auto __dupe_mask = _mm_setr_epi8(0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12);
				const auto __add_seq = _mm_setr_epi8(0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3);

				const auto __byte_offsets = _mm_slli_epi32(__as<__m128i>(__idx), 2);
				const auto __duplicated = _mm_shuffle_epi8(__byte_offsets, __dupe_mask);
				const auto __pshufb_mask = _mm_add_epi8(__duplicated, __add_seq);

				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __pshufb_mask));
			}
			else {
				alignas(16) _Type_ __t[4];
				_Store<_ISA_>()(__t, __x, __aligned_policy{});

				const auto* __i = reinterpret_cast<const unsigned int*>(&__idx);
				
				if constexpr (std::is_floating_point_v<_Type_>) return _Fallback_result{ __as<_Intrin_>(_mm_set_ps(__t[__i[3]], __t[__i[2]], __t[__i[1]], __t[__i[0]])) };
				else return _Fallback_result{ __as<_Intrin_>(_mm_set_epi32(__t[__i[3]], __t[__i[2]], __t[__i[1]], __t[__i[0]])) };
			}
		}
		else if constexpr (sizeof(_Type_) == 2) {
			if constexpr (__avx512vl && __avx512bw) {
				return __as<_Intrin_>(_mm_permutexvar_epi16(__as<__m128i>(__idx), __as<__m128i>(__x)));
			}
			else if constexpr (__ssse3) {
				const auto __dupe_mask = _mm_setr_epi8(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
				const auto __add_seq = _mm_setr_epi8(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1);

				const auto __byte_offsets = _mm_slli_epi16(__as<__m128i>(__idx), 1);
				const auto __duplicated = _mm_shuffle_epi8(__byte_offsets, __dupe_mask);
				const auto __pshufb_mask = _mm_add_epi8(__duplicated, __add_seq);

				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __pshufb_mask));
			}
			else {
				alignas(16) _Type_ __t[8];
				_Store<_ISA_>()(__t, __x, __aligned_policy{});

				const auto* __i = reinterpret_cast<const unsigned short*>(&__idx);
				return _Fallback_result{ __as<_Intrin_>(_mm_set_epi16(__t[__i[7]], __t[__i[6]], __t[__i[5]],
					__t[__i[4]], __t[__i[3]], __t[__i[2]], __t[__i[1]], __t[__i[0]])) };
			}
		}
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (__ssse3) {
				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __as<__m128i>(__idx)));
			}
			else {
				alignas(16) _Type_ __t[16];
				_Store<_ISA_>()(__t, __x, __aligned_policy{});

				const auto* __i = reinterpret_cast<const unsigned char*>(&__idx);
				return _Fallback_result { __as<_Intrin_>(_mm_set_epi8(__t[__i[15]], __t[__i[14]], __t[__i[13]],
					__t[__i[12]], __t[__i[11]], __t[__i[10]], __t[__i[9]], __t[__i[8]],
					__t[__i[7]], __t[__i[6]], __t[__i[5]], __t[__i[4]], __t[__i[3]],
					__t[__i[2]], __t[__i[1]], __t[__i[0]])) };
			}
		}
	}
	else if constexpr (sizeof(_Intrin_) == 32) {
		if constexpr (sizeof(_Type_) == 8) {
			if constexpr (__avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi64(__as<__m256i>(__idx), __as<__m256i>(__x)));
			}
			else if constexpr (__avx2) {
				const auto __dupe_mask = _mm256_setr_epi32(0, 0, 2, 2, 4, 4, 6, 6);
				const auto __add_seq = _mm256_setr_epi32(0, 1, 0, 1, 0, 1, 0, 1);

				const auto __byte_offsets = _mm256_slli_epi64(__as<__m256i>(__idx), 1);
				const auto __duplicated = _mm256_permutevar8x32_epi32(__byte_offsets, __dupe_mask);
				const auto __vpermd_indices = _mm256_add_epi32(__duplicated, __add_seq);

				return __as<_Intrin_>(_mm256_permutevar8x32_epi32(__as<__m256i>(__x), __vpermd_indices));
			}
			else return __shuffle_fallback<_ISA_, _Type_>(__x, __idx);
		}
		else if constexpr (sizeof(_Type_) == 4 && __avx2) {
			return __as<_Intrin_>(_mm256_permutevar8x32_epi32(__as<__m256i>(__x), __as<__m256i>(__idx)));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			if constexpr (__avx512vl && __avx512bw) {
				return __as<_Intrin_>(_mm256_permutexvar_epi16(__as<__m256i>(__idx), __as<__m256i>(__x)));
			}
			else if constexpr (__avx2) {
				const auto __byte_shuffle_mask = __byte_shuffle_mask_avx2<_ISA_>(__idx);

				// If the index in __byte_shuffle_mask is > 15, then it is located at the top of the ymm register 
				const auto __is_upper_half = _mm256_cmpgt_epi8(__byte_shuffle_mask, _mm256_set1_epi8(15));

				// __upper: {__x[2], __x[3], __x[2], __x[3]}
				const auto __upper = _mm256_shuffle_epi8(_mm256_permute4x64_epi64(__as<__m256i>(__x), 0xEE), __byte_shuffle_mask);
				// __lower: {__x[0], __x[1], __x[0], __x[1]}
				const auto __lower = _mm256_shuffle_epi8(_mm256_inserti128_si256(__as<__m256i>(__x), __as<__m128i>(__x), 1), __byte_shuffle_mask);

				return __as<_Intrin_>(_Select<_ISA_, _Type_>()(__upper, __lower, __is_upper_half));
			}
			else return __shuffle_fallback<_ISA_, _Type_>(__x, __idx);
		}
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (__avx512vl && __avx512vbmi) {
				return __as<_Intrin_>(_mm256_permutexvar_epi8(__as<__m256i>(__idx), __as<__m256i>(__x)));
			}
			else if constexpr (__avx2) {
				const auto __is_upper_half = _mm256_cmpgt_epi8(__as<__m256i>(__idx), _mm256_set1_epi8(15));
				
				const auto __upper = _mm256_shuffle_epi8(_mm256_permute4x64_epi64(__as<__m256i>(__x), 0xEE), __as<__m256i>(__idx));
				const auto __lower = _mm256_shuffle_epi8(_mm256_inserti128_si256(__as<__m256i>(__x), __as<__m128i>(__x), 1), __as<__m256i>(__idx));

				return __as<_Intrin_>(_Select<_ISA_, _Type_>()(__upper, __lower, __is_upper_half));
			}
			else return __shuffle_fallback<_ISA_, _Type_>(__x, __idx);
		}
		else return __shuffle_fallback<_ISA_, _Type_>(__x, __idx);
	}
	else if constexpr (sizeof(_Intrin_) == 64) {
		if constexpr (sizeof(_Type_) == 8) return __as<_Intrin_>(_mm512_permutexvar_epi64(__as<__m512i>(__idx), __as<__m512i>(__x)));
		else if constexpr (sizeof(_Type_) == 4) return __as<_Intrin_>(_mm512_permutexvar_epi32(__as<__m512i>(__idx), __as<__m512i>(__x)));
		else if constexpr (sizeof(_Type_) == 2) return __shuffle_i16x32_avx512<_ISA_>(__x, __idx);
		else if constexpr (sizeof(_Type_) == 1) return __shuffle_i8x64_avx512<_ISA_>(__x, __idx);
	}
	else return __shuffle_fallback<_ISA_, _Type_>(__x, __idx);
}

template <class _Return_>
concept __is_fallback = requires { { _Return_::__fallback } -> std::convertible_to<bool>; } && _Return_::__fallback;

template <class _MaybeWrapped_>
raze_nodiscard raze_always_inline auto __unwrap_fallback(const _MaybeWrapped_& __x) noexcept {
	if constexpr (__is_fallback<_MaybeWrapped_>) return __x._data;
	else return __x;
}
template <class _Pattern_>
raze_always_inline raze_no_stack_protector auto __generic_shuffle_native_size(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Simd_ = pattern_vector_t<_Pattern_>;
	_Simd_ __result = __x;

	auto& __storage = __result.template __get<0>();
	
	using _Ret = decltype(__generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__storage), __p));

	if constexpr (__is_fallback<_Ret>) {
		__storage = __generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__storage), __p)._data;
		return _Fallback_result{ __result };
	}
	else {
		__storage = __generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__storage), __p);
		return __result;
	}
}

template <simd_type _Simd_, index_simd_type _Index_>
raze_always_inline raze_no_stack_protector auto __generic_shuffle_native_size(const _Simd_& __x, const _Index_& __idx) noexcept {
	_Simd_ __result = __x;

	auto& __storage = __result.template __get<0>();
	auto __idx_native = __storage_unwrap(__idx.template __get<0>());
	
	using _Ret = decltype(__generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__storage), __idx_native));

	if constexpr (__is_fallback<_Ret>) {
		__storage = __generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__storage), __idx_native)._data;
		return _Fallback_result{ __result };
	}
	else {
		__storage = __generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__storage), __idx_native);
		return __result;
	}
}

template <class _Pattern_>
raze_always_inline raze_no_stack_protector auto __generic_shuffle(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Simd_ = pattern_vector_t<_Pattern_>;

	if constexpr (__is_identity(__p)) return __x;
	else if constexpr (native<_Simd_>) return __generic_shuffle_native_size(__x, __p);
	else if constexpr (!has_any_scalar_chunks<_Simd_> && _Simd_::__chunks_count() == 2 && trivially_chunk_swappable<_Simd_>) {
		_Simd_ __dup_1;
		__dup_1.template __get<0>() = __x.template __get<0>();
		__dup_1.template __get<1>() = __x.template __get<0>();

		_Simd_ __dup_2;
		__dup_2.template __get<0>() = __x.template __get<1>();
		__dup_2.template __get<1>() = __x.template __get<1>();

		constexpr auto __new_p = __p.split();

		[&] <sizetype... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda {
			([&](auto __i) raze_always_inline_lambda {
				auto& __c1 = __dup_1.template __get<__i>();
				auto& __c2 = __dup_2.template __get<__i>();

				auto __c1_in = __storage_unwrap(__c1);
				auto __c2_in = __storage_unwrap(__c2);

				using _Chunk1 = std::remove_cvref_t<decltype(__c1)>;
				using _Chunk2 = std::remove_cvref_t<decltype(__c2)>;

				const auto __r1 = __generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__c1_in,
					make_shuffle_pattern<typename _Chunk1::as_simd, [&](auto __k) { return __new_p.first[__k] % (_Simd_::size() / 2); } > {});

				const auto __r2 = __generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__c2_in,
					make_shuffle_pattern<typename _Chunk2::as_simd, [&](auto __k) { return __new_p.second[__k] % (_Simd_::size() / 2); } > {});

				__c1 = __unwrap_fallback(__r1);
				__c2 = __unwrap_fallback(__r2);
				}(std::integral_constant<sizetype, _Indices_>{}), ...);
		}(std::make_integer_sequence<sizetype, _Simd_::__chunks_count()>{});

		const auto __mask = __p.to_mask([](auto __idx) { return __idx >= (_Simd_::size() / 2); });
		return vx::__select[__dup_1, __mask](__dup_2);
	}
	else return __generic_shuffle_scalar_fallback(__x, __p);
}

template <simd_type _Simd_, index_simd_type _Index_>
raze_always_inline raze_no_stack_protector auto __generic_shuffle(const _Simd_& __x, const _Index_& __idx) noexcept
	requires (index_type_for<_Index_, _Simd_>)
{
	if constexpr (native<_Simd_>) return __generic_shuffle_native_size(__x, __idx);
	else return __generic_shuffle_scalar_fallback(__x, __idx);
}

__RAZE_VX_NAMESPACE_END 
