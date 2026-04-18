#pragma once 

#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/compact/Compress.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Compress_store {
	template <
		class _IntrinType_,
		class _MaskType_,
		class _AlignmentPolicy_>
	raze_static_operator raze_always_inline _Type_* operator()(
		void*				__address,
		_MaskType_			__mask,
		_IntrinType_		__vector,
		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
			requires(__is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_>)
	{
		const auto __compressed = _Compress<_ISA_, _Width_, _Type_>()(__vector, __mask);
		_Store<_ISA_, _Width_>()(__address, __compressed.second, __alignment_policy);
		return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
	}
};

//template <class _Type_>
//struct _Compress_store<arch::ISA::AVX512F, 512, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
//			_mm512_mask_compressstoreu_epi64(__address, ~__mask, __as<__m512i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512F>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
//			_mm512_mask_compressstoreu_epi32(__address, ~__mask, __as<__m512i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512F>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_ps_v<_Type_>) {
//			_mm512_mask_compressstoreu_ps(__address, ~__mask, __as<__m512>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512F>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_pd_v<_Type_>) {
//			_mm512_mask_compressstoreu_pd(__address, ~__mask, __as<__m512d>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512F>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			const auto __compressed = _Compress<arch::ISA::AVX512F, 512, _Type_>()(__vector, __mask);
//			_Store<arch::ISA::AVX512F, 512>()(__address, __compressed.second, __alignment_policy);
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512F, 512, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_>
//struct _Compress_store<arch::ISA::AVX512VLF, 256, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
//			_mm256_mask_compressstoreu_epi64(__address, ~__mask, __as<__m256i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
//			_mm256_mask_compressstoreu_epi32(__address, ~__mask, __as<__m256i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_ps_v<_Type_>) {
//			_mm256_mask_compressstoreu_ps(__address, ~__mask, __as<__m256>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_pd_v<_Type_>) {
//			_mm256_mask_compressstoreu_pd(__address, ~__mask, __as<__m256d>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			const auto __compressed = _Compress<arch::ISA::AVX512VLF, 256, _Type_>()(__vector, __mask);
//			_Store<arch::ISA::AVX512VLF, 256>()(__address, __compressed.second, __alignment_policy);
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VLF, 256, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_>
//struct _Compress_store<arch::ISA::AVX512VLF, 128, _Type_> :
//	_Compress_store<arch::ISA::AVX2, 128, _Type_>
//{
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_>) {
//			_mm_mask_compressstoreu_epi64(__address, ~__mask, __as<__m128i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_>) {
//			_mm_mask_compressstoreu_epi32(__address, ~__mask, __as<__m128i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_ps_v<_Type_>) {
//			_mm_mask_compressstoreu_ps(__address, ~__mask, __as<__m128>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_pd_v<_Type_>) {
//			_mm_mask_compressstoreu_pd(__address, ~__mask, __as<__m128d>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VLF>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			const auto __compressed = _Compress<arch::ISA::AVX512VLF, 128, _Type_>()(__vector, __mask);
//			_Store<arch::ISA::AVX512VLF, 128>()(__address, __compressed.second, __alignment_policy);
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __compressed.first));
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VLF, 128, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_> 
//struct _Compress_store<arch::ISA::AVX512VBMI2, 512, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
//			_mm512_mask_compressstoreu_epi16(__address, ~__mask, __as<__m512i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
//			_mm512_mask_compressstoreu_epi8(__address, ~__mask, __as<__m512i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			return _Compress_store<arch::ISA::AVX512F, 512, _Type_>()(
//				__address, __mask, __vector, __alignment_policy);
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VBMI2, 512, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_> 
//struct _Compress_store<arch::ISA::AVX512VBMI2DQ, 512, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
//			_mm512_mask_compressstoreu_epi16(__address, ~__mask, __as<__m512i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2DQ>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
//			_mm512_mask_compressstoreu_epi8(__address, ~__mask, __as<__m512i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2DQ>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			return _Compress_store<arch::ISA::AVX512F, 512, _Type_>()(
//				__address, __mask, __vector, __alignment_policy);
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VBMI2DQ, 512, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_> 
//struct _Compress_store<arch::ISA::AVX512VBMI2VL, 256, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
//			_mm256_mask_compressstoreu_epi16(__address, ~__mask, __as<__m256i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VL>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
//			_mm256_mask_compressstoreu_epi8(__address, ~__mask, __as<__m256i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VL>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			return _Compress_store<arch::ISA::AVX512VLF, 256, _Type_>()(
//				__address, __mask, __vector, __alignment_policy);
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VBMI2VL, 256, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_> 
//struct _Compress_store<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
//			_mm256_mask_compressstoreu_epi16(__address, ~__mask, __as<__m256i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VLDQ>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
//			_mm256_mask_compressstoreu_epi8(__address, ~__mask, __as<__m256i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VLDQ>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			return _Compress_store<arch::ISA::AVX512VBMI2VL, 256, _Type_>()(
//				__address, __mask, __vector, __alignment_policy);
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_> 
//struct _Compress_store<arch::ISA::AVX512VBMI2VL, 128, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
//			_mm_mask_compressstoreu_epi16(__address, ~__mask, __as<__m128i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VL>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
//			_mm_mask_compressstoreu_epi8(__address, ~__mask, __as<__m128i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VL>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			return _Compress_store<arch::ISA::AVX512VLF, 128, _Type_>()(
//				__address, __mask, __vector, __alignment_policy);
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VBMI2VL, 128, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};
//
//template <class _Type_> 
//struct _Compress_store<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> {
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(std::is_integral_v<_MaskType_>)
//	{
//		if constexpr (__is_epi16_v<_Type_> || __is_epu16_v<_Type_>) {
//			_mm_mask_compressstoreu_epi16(__address, ~__mask, __as<__m128i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VLDQ>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else if constexpr (__is_epi8_v<_Type_> || __is_epu8_v<_Type_>) {
//			_mm_mask_compressstoreu_epi8(__address, ~__mask, __as<__m128i>(__vector));
//			const auto __offset = math::__popcnt_population_count(__to_gpr<arch::ISA::AVX512VBMI2VLDQ>(__mask));
//			return reinterpret_cast<_Type_*>(algorithm::__bytes_pointer_offset(__address, __offset * sizeof(_Type_)));
//		}
//		else {
//			return _Compress_store<arch::ISA::AVX512VBMI2VL, 128, _Type_>()(
//				__address, __mask, __vector, __alignment_policy);
//		}
//	}
//
//	template <
//		class _IntrinType_,
//		class _MaskType_,
//		class _AlignmentPolicy_>
//	raze_static_operator raze_always_inline _Type_* operator()(
//		void*				__address,
//		_MaskType_			__mask,
//		_IntrinType_		__vector,
//		_AlignmentPolicy_&& __alignment_policy = _AlignmentPolicy_{}) raze_const_operator noexcept
//			requires(__is_intrin_type_v<_MaskType_>)
//	{
//		return (*this)(__address, _To_mask<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_>()(
//			__mask), __vector, __alignment_policy);
//	}
//};


__RAZE_VX_NAMESPACE_END
