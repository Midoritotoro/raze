#pragma once 

#include <src/raze/vx/memory/Load.h>
#include <src/raze/vx/memory/Store.h>
#include <src/raze/vx/shuffle/Blend.h>
#include <src/raze/vx/bitwise/MaskConvert.h>
#include <src/raze/vx/memory/MaskStoreIntrin.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Mask_store;

template <class _Type_>
struct _Mask_store<arch::ISA::SSE2, 128, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&&	__alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		_Store<arch::ISA::SSE2, 128>()(__address, _Blend<arch::ISA::SSE2, 128, _Type_>()(
			__vector, _Load<arch::ISA::SSE2, 128, _IntrinType_>()(
				__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _Type_> struct _Mask_store<arch::ISA::SSE3, 128, _Type_> : _Mask_store<arch::ISA::SSE2, 128, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::SSSE3, 128, _Type_> : _Mask_store<arch::ISA::SSE3, 128, _Type_> {};

template <class _Type_> 
struct _Mask_store<arch::ISA::SSE41, 128, _Type_>:
	_Mask_store<arch::ISA::SSSE3, 128, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		_Store<arch::ISA::SSE41, 128>()(__address, _Blend<arch::ISA::SSE41, 128, _Type_>()(
			__vector, _Load<arch::ISA::SSE41, 128, _IntrinType_>()(
				__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _Type_> struct _Mask_store<arch::ISA::SSE42, 128, _Type_> : _Mask_store<arch::ISA::SSE41, 128, _Type_> {};

template <class _Type_> 
struct _Mask_store<arch::ISA::AVX, 128, _Type_>: 
	_Mask_store<arch::ISA::SSE42, 128, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_Type_>) {
			__vmaskmovpd(reinterpret_cast<double*>(__address),
				_To_vector<arch::ISA::AVX, 128, __m128i, _Type_>()(__mask),
				__as<__m128d>(__vector));
		}
		else if constexpr (__is_ps_v<_Type_>) {
			__vmaskmovps(reinterpret_cast<float*>(__address),
				_To_vector<arch::ISA::AVX, 128, __m128i, _Type_>()(__mask),
				__as<__m128>(__vector));
		}
		else {
			_Store<arch::ISA::AVX, 128>()(__address, _Blend<arch::ISA::AVX, 128, _Type_>()(
				__vector, _Load<arch::ISA::AVX, 128, _IntrinType_>()(
					__address, __alignment_policy), __mask), __alignment_policy);
		}
	}
};

template <class _Type_> struct _Mask_store<arch::ISA::FMA3, 128, _Type_> : _Mask_store<arch::ISA::AVX, 128, _Type_> {};


template <class _Type_> 
struct _Mask_store<arch::ISA::AVX2, 128, _Type_>: 
	_Mask_store<arch::ISA::AVX, 128, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			__vpmaskmovq(reinterpret_cast<long long*>(__address),
				_To_vector<arch::ISA::AVX2, 128, __m128i, _Type_>()(__mask),
				__as<__m128i>(__vector));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			__vpmaskmovd(reinterpret_cast<int*>(__address),
				_To_vector<arch::ISA::AVX2, 128, __m128i, _Type_>()(__mask),
				__as<__m128i>(__vector));

		else
			return _Mask_store<arch::ISA::AVX, 128, _Type_>()(
				__address, __mask, __vector, __alignment_policy);
	}
};

template <class _Type_> struct _Mask_store<arch::ISA::AVX2FMA3, 128, _Type_> : _Mask_store<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_>
struct _Mask_store<arch::ISA::AVX, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_pd_v<_Type_>)
			__vmaskmovpd(reinterpret_cast<double*>(__address),
				_To_vector<arch::ISA::AVX, 256, __m256i, _Type_>()(__mask),
				__as<__m256d>(__vector));

		else if constexpr (__is_ps_v<_Type_>)
			__vmaskmovps(reinterpret_cast<float*>(__address),
				_To_vector<arch::ISA::AVX, 256, __m256i, _Type_>()(__mask),
				__as<__m256>(__vector));

		else
			_Store<arch::ISA::AVX, 256>()(__address, _Blend<arch::ISA::AVX, 256, _Type_>()(
				__vector, _Load<arch::ISA::AVX, 256, _IntrinType_>()(
					__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _Type_> struct _Mask_store<arch::ISA::FMA3, 256, _Type_> : _Mask_store<arch::ISA::AVX, 256, _Type_> {};

template <class _Type_>
struct _Mask_store<arch::ISA::AVX2, 256, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			__vpmaskmovq(reinterpret_cast<long long*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask),
				__as<__m256i>(__vector));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			__vpmaskmovd(reinterpret_cast<int*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask),
				__as<__m256i>(__vector));

		else if constexpr (__is_pd_v<_Type_>)
			__vmaskmovpd(reinterpret_cast<double*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask),
				__as<__m256d>(__vector));

		else if constexpr (__is_ps_v<_Type_>)
			__vmaskmovps(reinterpret_cast<float*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask),
				__as<__m256>(__vector));

		else
			_Store<arch::ISA::AVX2, 256>()(__address, _Blend<arch::ISA::AVX2, 256, _Type_>()(
				__vector, _Load<arch::ISA::AVX2, 256, _IntrinType_>()(
					__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _Type_> struct _Mask_store<arch::ISA::AVX2FMA3, 256, _Type_> : _Mask_store<arch::ISA::AVX2, 256, _Type_> {};

template <class _Type_>
struct _Mask_store<arch::ISA::AVX512F, 512, _Type_> {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
				return _mm512_mask_store_epi64(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512i>(__vector));

			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
				return _mm512_mask_store_epi32(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512i>(__vector));

			else if constexpr (__is_pd_v<_Type_>)
				return _mm512_mask_store_pd(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512d>(__vector));

			else if constexpr (__is_ps_v<_Type_>)
				return _mm512_mask_store_ps(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512>(__vector));
		}
		else {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
				return _mm512_mask_storeu_epi64(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512i>(__vector));

			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
				return _mm512_mask_storeu_epi32(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512i>(__vector));

			else if constexpr (__is_pd_v<_Type_>)
				return _mm512_mask_storeu_pd(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512d>(__vector));

			else if constexpr (__is_ps_v<_Type_>)
				return _mm512_mask_storeu_ps(__address, _To_mask<arch::ISA::AVX512F, 512,
					_Type_>()(__mask), __as<__m512>(__vector));
		}

		_Store<arch::ISA::AVX512F, 512>()(__address, _Blend<arch::ISA::AVX512F, 512, _Type_>()(__vector, 
			_Load<arch::ISA::AVX512F, 512, _IntrinType_>()(
				__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _Type_>
struct _Mask_store<arch::ISA::AVX512BW, 512, _Type_>: 
	_Mask_store<arch::ISA::AVX512F, 512, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return _mm512_mask_storeu_epi16(__address, _To_mask<arch::ISA::AVX512BW, 512,
				_Type_>()(__mask), __as<__m512i>(__vector));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return _mm512_mask_storeu_epi8(__address, _To_mask<arch::ISA::AVX512BW, 512,
				_Type_>()(__mask), __as<__m512i>(__vector));

		else
			return _Mask_store<arch::ISA::AVX512F, 512, _Type_>()(__address, __mask, __vector, __alignment_policy);
	}
};

template <class _Type_>
struct _Mask_store<arch::ISA::AVX512VLF, 256, _Type_>: 
	_Mask_store<arch::ISA::AVX2, 256, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Mask_store<arch::ISA::AVX2, 256, _Type_>()(__address, __mask, __vector, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return _mm256_mask_store_epi64(__address,
						__mask, __as<__m256i>(__vector));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return _mm256_mask_store_epi32(__address,
						__mask, __as<__m256i>(__vector));

				else if constexpr (__is_pd_v<_Type_>)
					return _mm256_mask_store_pd(__address, 
						__mask, __as<__m256d>(__vector));

				else if constexpr (__is_ps_v<_Type_>)
					return _mm256_mask_store_ps(__address, 
						__mask, __as<__m256>(__vector));
			}
			else {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return _mm256_mask_storeu_epi64(__address,
						__mask, __as<__m256i>(__vector));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return _mm256_mask_storeu_epi32(__address, 
						__mask, __as<__m256i>(__vector));

				else if constexpr (__is_pd_v<_Type_>)
					return _mm256_mask_storeu_pd(__address, 
						__mask, __as<__m256d>(__vector));

				else if constexpr (__is_ps_v<_Type_>)
					return _mm256_mask_storeu_ps(__address,
						__mask, __as<__m256>(__vector));
			}
		}

		_Store<arch::ISA::AVX512VLF, 256>()(__address, _Blend<arch::ISA::AVX512VLF, 256, _Type_>()(__vector,
			_Load<arch::ISA::AVX512VLF, 256, _IntrinType_>()(
				__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _Type_> 
struct _Mask_store<arch::ISA::AVX512VLF, 128, _Type_>: 
	_Mask_store<arch::ISA::AVX2, 128, _Type_> 
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Mask_store<arch::ISA::AVX2, 128, _Type_>()(__address, __mask, __vector, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return _mm_mask_store_epi64(__address,
						__mask, __as<__m128i>(__vector));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return _mm_mask_store_epi32(__address, 
						__mask, __as<__m128i>(__vector));

				else if constexpr (__is_pd_v<_Type_>)
					return _mm_mask_store_pd(__address,
						__mask, __as<__m128d>(__vector));

				else if constexpr (__is_ps_v<_Type_>)
					return _mm_mask_store_ps(__address, 
						__mask, __as<__m128>(__vector));
			}
			else {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return _mm_mask_storeu_epi64(__address,
						__mask, __as<__m128i>(__vector));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return _mm_mask_storeu_epi32(__address, 
						__mask, __as<__m128i>(__vector));

				else if constexpr (__is_pd_v<_Type_>)
					return _mm_mask_storeu_pd(__address, 
						__mask, __as<__m128d>(__vector));

				else if constexpr (__is_ps_v<_Type_>)
					return _mm_mask_storeu_ps(__address,
						__mask, __as<__m128>(__vector));
			}
		}

		_Store<arch::ISA::AVX512VLF, 128>()(__address, _Blend<arch::ISA::AVX512VLF, 128, _Type_>()(__vector,
			_Load<arch::ISA::AVX512VLF, 128, _IntrinType_>()(
				__address, __alignment_policy), __mask), __alignment_policy);
	}
};

template <class _Type_>
struct _Mask_store<arch::ISA::AVX512VLBW, 128, _Type_>:
	_Mask_store<arch::ISA::AVX512VLF, 128, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) == 2)
			return _mm_mask_storeu_epi16(__address, _To_mask<arch::ISA::AVX512VLBW, 128,
				_Type_>()(__mask), __as<__m128i>(__vector));

		else if constexpr (sizeof(_Type_) == 1)
			return _mm_mask_storeu_epi8(__address, _To_mask<arch::ISA::AVX512VLBW, 128, 
				_Type_>()(__mask), __as<__m128i>(__vector));

		else
			return _Mask_store<arch::ISA::AVX512VLF, 128, _Type_>()(
				__address, __mask, __vector, __alignment_policy);
	}
};

template <class _Type_>
struct _Mask_store<arch::ISA::AVX512VLBW, 256, _Type_> :
	_Mask_store<arch::ISA::AVX512VLF, 256, _Type_>
{
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_static_operator raze_always_inline void operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) == 2)
			return _mm256_mask_storeu_epi16(__address, _To_mask<arch::ISA::AVX512VLBW, 256,
				_Type_>()(__mask), __as<__m256i>(__vector));

		else if constexpr (sizeof(_Type_) == 1)
			return _mm256_mask_storeu_epi8(__address, _To_mask<arch::ISA::AVX512VLBW, 256,
				_Type_>()(__mask), __as<__m256i>(__vector));

		else
			return _Mask_store<arch::ISA::AVX512VLF, 256, _Type_>()(
				__address, __mask, __vector, __alignment_policy);
	}
};

template <class _Type_> struct _Mask_store<arch::ISA::AVX512DQ, 512, _Type_> : _Mask_store<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512BWDQ, 512, _Type_> : _Mask_store<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMI, 512, _Type_> : _Mask_store<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMI2, 512, _Type_> : _Mask_store<arch::ISA::AVX512VBMI, 512, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Mask_store<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Mask_store<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};
;
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VLDQ, 256, _Type_> : _Mask_store<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Mask_store<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Mask_store<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMI2VL, 256, _Type_> : _Mask_store<arch::ISA::AVX512VBMIVL, 256, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Mask_store<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Mask_store<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Mask_store<arch::ISA::AVX512VLDQ, 128, _Type_> : _Mask_store<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Mask_store<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Mask_store<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMI2VL, 128, _Type_> : _Mask_store<arch::ISA::AVX512VBMIVL, 128, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Mask_store<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Mask_store<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Mask_store<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
