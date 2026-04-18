#pragma once 

#include <src/raze/vx/hw/x86/memory/MaskLoad.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_,
	class		_IntrinType_>
struct _Maskz_load;

template <	
	class _Type_,
	class _IntrinType_>
struct _Maskz_load<arch::ISA::SSE2, 128, _Type_, _IntrinType_> {
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&&	__alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::SSE2, 128, _Type_>()(
			_Load<arch::ISA::SSE2, 128, _IntrinType_>()(__address), __mask);
	}
};

template <
	class _Type_,
	class _IntrinType_>
struct _Maskz_load<arch::ISA::SSE3, 128, _Type_, _IntrinType_>:
	_Maskz_load<arch::ISA::SSE2, 128, _Type_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::SSE3, 128, _Type_>()(
			_Load<arch::ISA::SSE3, 128, _IntrinType_>()(__address), __mask);
	}
};

template <
	class _Type_,
	class _IntrinType_> 
struct _Maskz_load<arch::ISA::SSE41, 128, _Type_, _IntrinType_>:
	_Maskz_load<arch::ISA::SSSE3, 128, _Type_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		return _Maskz_assign<arch::ISA::SSE41, 128, _Type_>()(
			_Load<arch::ISA::SSE41, 128, _IntrinType_>()(__address), __mask);
	}
};

template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::SSSE3, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::SSE3, 128, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::SSE42, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::SSE41, 128, _Type_, _IntrinType_> {};

template <
	class _Type_,
	class _IntrinType_> 
struct _Maskz_load<arch::ISA::AVX, 128, _Type_, _IntrinType_>: 
	_Maskz_load<arch::ISA::SSE42, 128, _Type_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskload_ps(
				reinterpret_cast<const float*>(__address),
				_To_vector<arch::ISA::AVX, 128, __m128i, _Type_>()(__mask)));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskload_pd(
				reinterpret_cast<const double*>(__address),
				_To_vector<arch::ISA::AVX, 128, __m128i, _Type_>()(__mask)));

		else
			return _Maskz_assign<arch::ISA::AVX, 128, _Type_>()(
				_Load<arch::ISA::AVX, 128, _IntrinType_>()(__address), __mask);
	}
};

template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::FMA3, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX, 128, _Type_, _IntrinType_> {};

template <
	class _Type_,
	class _IntrinType_>
struct _Maskz_load<arch::ISA::AVX2, 128, _Type_, _IntrinType_>:
	_Maskz_load<arch::ISA::AVX, 128, _Type_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskload_epi64(
				reinterpret_cast<const long long*>(__address),
				_To_vector<arch::ISA::AVX2, 128, __m128i, _Type_>()(__mask)));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskload_epi32(
				reinterpret_cast<const int*>(__address),
				_To_vector<arch::ISA::AVX2, 128, __m128i, _Type_>()(__mask)));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskload_ps(
				reinterpret_cast<const float*>(__address),
				_To_vector<arch::ISA::AVX2, 128, __m128i, _Type_>()(__mask)));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm_maskload_ps(
				reinterpret_cast<const float*>(__address),
				_To_vector<arch::ISA::AVX2, 128, __m128i, _Type_>()(__mask)));

		else
			return _Maskz_assign<arch::ISA::AVX2, 128, _Type_>()(
				_Load<arch::ISA::AVX2, 128, _IntrinType_>()(__address), __mask);
	}
};

template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX2FMA3, 128, _Type_, _IntrinType_>: _Maskz_load<arch::ISA::AVX2, 128, _Type_, _IntrinType_> {};

template <
	class _Type_,
	class _IntrinType_> 
struct _Maskz_load<arch::ISA::AVX, 256, _Type_, _IntrinType_> {
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskload_ps(
				reinterpret_cast<const float*>(__address),
				_To_vector<arch::ISA::AVX, 256, __m256i, _Type_>()(__mask)));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskload_ps(
				reinterpret_cast<const float*>(__address),
				_To_vector<arch::ISA::AVX, 256, __m256i, _Type_>()(__mask)));

		else
			return _Maskz_assign<arch::ISA::AVX, 256, _Type_>()(
				_Load<arch::ISA::AVX, 256, _IntrinType_>()(__address), __mask);
	}
};

template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::FMA3, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX, 256, _Type_, _IntrinType_> {};

template <
	class _Type_,
	class _IntrinType_>
struct _Maskz_load<arch::ISA::AVX2, 256, _Type_, _IntrinType_>:
	_Maskz_load<arch::ISA::AVX, 256, _Type_, _IntrinType_> 
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskload_epi64(
				reinterpret_cast<const long long*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask)));

		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskload_epi32(
				reinterpret_cast<const int*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask)));

		else if constexpr (__is_ps_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskload_ps(
				reinterpret_cast<const float*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask)));

		else if constexpr (__is_pd_v<_Type_>)
			return __as<_IntrinType_>(_mm256_maskload_ps(
				reinterpret_cast<const float*>(__address),
				_To_vector<arch::ISA::AVX2, 256, __m256i, _Type_>()(__mask)));

		else
			return _Maskz_assign<arch::ISA::AVX2, 256, _Type_>()(
				_Load<arch::ISA::AVX2, 256, _IntrinType_>()(__address), __mask);
	}
};

template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX2FMA3, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX2, 256, _Type_, _IntrinType_> {};

template <
	class _Type_,
	class _IntrinType_>
struct _Maskz_load<arch::ISA::AVX512F, 512, _Type_, _IntrinType_> {
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_load_epi64(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_load_epi32(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_pd_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_load_pd(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_ps_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_load_ps(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));
		}
		else {
			if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_loadu_epi64(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_loadu_epi32(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_pd_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_loadu_pd(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));

			else if constexpr (__is_ps_v<_Type_>)
				return __as<_IntrinType_>(_mm512_maskz_loadu_ps(
					_To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask), __address));
		}

		return _Maskz_assign<arch::ISA::AVX512F, 512, _Type_>()(
			_Load<arch::ISA::AVX512F, 512, _IntrinType_>()(__address), __mask);
	}
};

template <
	class _Type_,
	class _IntrinType_> 
struct _Maskz_load<arch::ISA::AVX512BW, 512, _Type_, _IntrinType_>:
	_Maskz_load<arch::ISA::AVX512F, 512, _Type_, _IntrinType_> 
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_loadu_epi16(
				_To_mask<arch::ISA::AVX512BW, 512, _Type_>()(__mask), __address));

		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>)
			return __as<_IntrinType_>(_mm512_maskz_loadu_epi8(
				_To_mask<arch::ISA::AVX512BW, 512, _Type_>()(__mask), __address));

		else
			return _Maskz_load<arch::ISA::AVX512F, 512, _Type_, _IntrinType_>()(
				__address, __mask, __alignment_policy);
	}
};

template <
	class _Type_,
	class _IntrinType_>
struct _Maskz_load<arch::ISA::AVX512VLF, 256, _Type_, _IntrinType_>:
	_Maskz_load<arch::ISA::AVX2, 256, _Type_, _IntrinType_> 
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Maskz_load<arch::ISA::AVX2, 256, _Type_, _IntrinType_>()(__address, __mask, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_load_epi64(__mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_load_epi32(__mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_load_pd(__mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_load_ps(__mask, __address));
			}
			else {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_loadu_epi64(__mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_loadu_epi32(__mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_loadu_pd(__mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm256_maskz_loadu_ps(__mask, __address));
			}
		}

		return _Maskz_assign<arch::ISA::AVX512VLF, 256, _Type_>()(
			_Load<arch::ISA::AVX512VLF, 256, _IntrinType_>()(__address), __mask);
	}
};

template <
	class _Type_,
	class _IntrinType_> 
struct _Maskz_load<arch::ISA::AVX512VLBW, 256, _Type_, _IntrinType_>: 
	_Maskz_load<arch::ISA::AVX512VLF, 256, _Type_, _IntrinType_> 
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) == 2)
			return __as<_IntrinType_>(_mm256_maskz_loadu_epi16(
				_To_mask<arch::ISA::AVX512VLBW, 256, _Type_>()(__mask), __address));

		else if constexpr (sizeof(_Type_) == 1)
			return __as<_IntrinType_>(_mm256_maskz_loadu_epi8(
				_To_mask<arch::ISA::AVX512VLBW, 256, _Type_>()(__mask), __address));

		else
			return _Maskz_load<arch::ISA::AVX512VLF, 256, _Type_, _IntrinType_>()(
				__address, __mask, __alignment_policy);
	}
};

template <
	class _Type_,
	class _IntrinType_>
struct _Maskz_load<arch::ISA::AVX512VLF, 128, _Type_, _IntrinType_>:
	_Maskz_load<arch::ISA::AVX2, 128, _Type_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (__is_intrin_type_v<_MaskType_>) {
			return _Maskz_load<arch::ISA::AVX2, 128, _Type_, _IntrinType_>()(__address, __mask, __alignment_policy);
		}
		else {
			if constexpr (std::remove_cvref_t<_AlignmentPolicy_>::__alignment) {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_load_epi64(__mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_load_epi32(__mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_load_pd(__mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_load_ps(__mask, __address));
			}
			else {
				if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_loadu_epi64(__mask, __address));

				else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_loadu_epi32(__mask, __address));

				else if constexpr (__is_pd_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_loadu_pd(__mask, __address));

				else if constexpr (__is_ps_v<_Type_>)
					return __as<_IntrinType_>(_mm_maskz_loadu_ps(__mask, __address));
			}
		}

		return _Maskz_assign<arch::ISA::AVX512VLF, 128, _Type_>()(
			_Load<arch::ISA::AVX512VLF, 128, _IntrinType_>()(__address), __mask);
	}
};

template <
	class _Type_,
	class _IntrinType_> 
struct _Maskz_load<arch::ISA::AVX512VLBW, 128, _Type_, _IntrinType_>: 
	_Maskz_load<arch::ISA::AVX512VLF, 128, _Type_, _IntrinType_>
{
	template <
		class _MaskType_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_nodiscard raze_static_operator raze_always_inline _IntrinType_ operator()(
		const void*			__address,
		_MaskType_			__mask,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(std::is_integral_v<_MaskType_> || __is_intrin_type_v<_MaskType_>)
	{
		if constexpr (sizeof(_Type_) == 2)
			return __as<_IntrinType_>(_mm_maskz_loadu_epi16(
				_To_mask<arch::ISA::AVX512VLBW, 128, _Type_>()(__mask), __address));

		else if constexpr (sizeof(_Type_) == 1)
			return __as<_IntrinType_>(_mm_maskz_loadu_epi8(
				_To_mask<arch::ISA::AVX512VLBW, 128, _Type_>()(__mask), __address));

		else
			return _Maskz_load<arch::ISA::AVX512VLF, 128, _Type_, _IntrinType_>()(
				__address, __mask, __alignment_policy);
	}
};

template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512DQ, 512, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512F, 512, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512BWDQ, 512, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512BW, 512, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMI, 512, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512BW, 512, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMI2, 512, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VBMI, 512, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMIDQ, 512, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512BWDQ, 512, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMI2DQ, 512, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VBMIDQ, 512, _Type_, _IntrinType_> {};

template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VLDQ, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLF, 256, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VLBWDQ, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLBW, 256, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMIVL, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLBW, 256, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMI2VL, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VBMIVL, 256, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMIVLDQ, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLBWDQ, 256, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VBMIVLDQ, 256, _Type_, _IntrinType_> {};


template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VLDQ, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLF, 128, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VLBWDQ, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLBW, 128, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMIVL, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLBW, 128, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMI2VL, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VBMIVL, 128, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMIVLDQ, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VLBWDQ, 128, _Type_, _IntrinType_> {};
template <class _Type_, class _IntrinType_> struct _Maskz_load<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_, _IntrinType_> : _Maskz_load<arch::ISA::AVX512VBMIVLDQ, 128, _Type_, _IntrinType_> {};

__RAZE_VX_NAMESPACE_END
