#pragma once 

#include <raze/arch/CpuFeature.h>

#include <src/raze/type_traits/TypeCheck.h>
#include <src/raze/type_traits/IsVirtualBaseOf.h>
#include <src/raze/type_traits/FunctionInformation.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    int32 __first_,
    int32 _Second_>
static constexpr int32 __constexpr_max() noexcept {
    return (__first_ > _Second_) ? __first_ : _Second_;
}

template <
    arch::ISA	_ISA_,
    typename	_Element_,
    uint32		_Width_>
class simd;

template <arch::ISA _ISA_>
constexpr auto __default_width =
    arch::__is_xmm_v<_ISA_> ? 128 :
    arch::__is_ymm_v<_ISA_> ? 256 :
    arch::__is_zmm_v<_ISA_> ? 512 : -1;


template <arch::ISA _ISA_>
constexpr auto __vector_default_size = __default_width<_ISA_>;

template <
	arch::ISA	_ISA_, 
	uint32		_Width_> 
constexpr bool __is_width_for_generation_v = __vector_default_size<_ISA_> >= _Width_;

template <class _Type_>
constexpr bool __is_intrin_type_v = type_traits::is_any_of_v<std::remove_cvref_t<_Type_>,
	__m128, __m128i, __m128d, __m256, __m256i, __m256d, __m512, __m512i, __m512d>;

template <typename _Element_>
constexpr bool __is_epi64_v =  
	((std::is_signed_v<_Element_> && !std::is_floating_point_v<_Element_>) 
		|| std::is_pointer_v<_Element_>
		|| std::is_same_v<_Element_, std::nullptr_t>) && sizeof(_Element_) == 8;

template <typename _Element_>
constexpr bool __is_epu64_v = 
	((std::is_unsigned_v<_Element_> && !std::is_floating_point_v<_Element_>) 
		|| std::is_pointer_v<_Element_> 
		|| std::is_same_v<_Element_, std::nullptr_t>) && sizeof(_Element_) == 8;

template <typename _Element_>
constexpr bool __is_epi32_v = 
	((std::is_signed_v<_Element_> && !std::is_floating_point_v<_Element_>) 
		|| std::is_pointer_v<_Element_>
		|| std::is_same_v<_Element_, std::nullptr_t>) && sizeof(_Element_) == 4;

template <typename _Element_>
constexpr bool __is_epu32_v = 
	((std::is_unsigned_v<_Element_> && !std::is_floating_point_v<_Element_>)
		|| std::is_pointer_v<_Element_>
		|| std::is_same_v<_Element_, std::nullptr_t>) && sizeof(_Element_) == 4;

template <typename _Element_>
constexpr bool __is_epi16_v = sizeof(_Element_) == 2 && std::is_signed_v<_Element_> && !std::is_floating_point_v<_Element_>;

template <typename _Element_>
constexpr bool __is_epu16_v = sizeof(_Element_) == 2 && std::is_unsigned_v<_Element_> && !std::is_floating_point_v<_Element_>;

template <typename _Element_>
constexpr bool __is_epi8_v  = sizeof(_Element_) == 1 && std::is_signed_v<_Element_> && !std::is_floating_point_v<_Element_>;

template <typename _Element_>
constexpr bool __is_epu8_v  = sizeof(_Element_) == 1 && std::is_unsigned_v<_Element_> && !std::is_floating_point_v<_Element_>;

template <typename _Element_>
constexpr bool __is_pd_v    = sizeof(_Element_) == 8 && type_traits::is_any_of_v<_Element_, double, long double>;

template <typename _Element_>
constexpr bool __is_ps_v    = sizeof(_Element_) == 4 && std::is_same_v<_Element_, float>;

#if !defined(__verify_simd_width) 
#  define __verify_simd_width(_Generation_, _Width_) \
	static_assert(raze::datapar::__is_width_for_generation_v<_Generation_, _Width_>, "Simd generation does not support this register width. ");
#endif // !defined(__verify_simd_width)

template <
	class _BasicSimd_, 
	class = void>
struct __is_valid_simd: 
	std::false_type
{};

template <class _BasicSimd_>
struct __is_valid_simd<
    _BasicSimd_,
    std::void_t<simd<
        _BasicSimd_::__isa,
        typename _BasicSimd_::value_type,
        _BasicSimd_::__width>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            simd<_BasicSimd_::__isa,
                       typename _BasicSimd_::value_type,
                       _BasicSimd_::__width>,
            _BasicSimd_> ||
        std::is_same_v<
            simd<_BasicSimd_::__isa,
                       typename _BasicSimd_::value_type,
                       _BasicSimd_::__width>,
            _BasicSimd_>> 
{};

template <class _BasicSimd_>
constexpr bool __is_valid_simd_v = __is_valid_simd<_BasicSimd_>::value;

template <
    class	_VectorType_,
    bool	_IsSimd_	= __is_valid_simd_v<_VectorType_>,
    bool	_IsIntrin_	= __is_intrin_type_v<_VectorType_>>
struct __vector_element_t {
    using type = void;
};

template <class _VectorType_>
struct __vector_element_t<_VectorType_, false, true> {
    using type = std::conditional_t<type_traits::is_any_of_v<_VectorType_, __m128i, __m256i, __m512i>, int, 
		std::conditional_t<type_traits::is_any_of_v<_VectorType_, __m128d, __m256d, __m512d>, double, 
			std::conditional_t<type_traits::is_any_of_v<_VectorType_, __m128, __m256, __m512>, float, void>>>;
};

template <class _VectorType_>
struct __vector_element_t<_VectorType_, true, false> {
    using type = typename _VectorType_::value_type;
};

template <class _VectorType_>
using __vector_element_type = typename __vector_element_t<_VectorType_>::type;

template <
	class _VectorType_,
	bool	_IsIntrin_	= __is_intrin_type_v<_VectorType_>,
	bool	_IsSimd_	= __is_valid_simd_v<_VectorType_>>
struct __unwrapped_vector_t {
	using type = _VectorType_;
};

template <class _VectorType_>
struct __unwrapped_vector_t<_VectorType_, false, true> {
	using type = typename _VectorType_::vector_type;
};

template <class _VectorType_>
struct __unwrapped_vector_t<_VectorType_, true, false> {
	using type = _VectorType_;
};

template <class _VectorType_>
using __unwrapped_vector_type = typename __unwrapped_vector_t<_VectorType_>::type;

template <
	class		_Simd_,
	typename	_ReturnType_,
	typename	_DesiredType_>
using __native_compare_return_type_helper = std::conditional_t<__is_intrin_type_v<_ReturnType_>,
	simd<_Simd_::__isa, _DesiredType_, _Simd_::__width>, _ReturnType_>;

template <
    class _RebindType_,
    class _VectorType_,
    bool _IsSimd_	= __is_valid_simd_v<_VectorType_>,
    bool _IsIntrin_ = __is_intrin_type_v<_VectorType_>>
struct __rebind_vector_element_t;

template <
    arch::ISA	_ToISA_,
    class       _RebindType_,
    class       _VectorType_,
    bool        _IsSimd_	= __is_valid_simd_v<_VectorType_>,
    bool        _IsIntrin_	= __is_intrin_type_v<_VectorType_>>
struct __rebind_vector_generation_t;

template <
    class _RebindType_,
    class _VectorType_>
using __rebind_vector_element_type = typename __rebind_vector_element_t<_RebindType_, _VectorType_>::type;

template <
    arch::ISA	_ToISA_,
    class       _RebindType_,
    class       _VectorType_>
using __rebind_vector_generation_type = typename __rebind_vector_generation_t<_ToISA_, _RebindType_, _VectorType_>::type;

template <arch::ISA _ISA_>
constexpr inline bool __has_sse2_support_v = static_cast<int>(_ISA_) >= static_cast<int>(arch::ISA::SSE2);

template <arch::ISA _ISA_>
constexpr inline bool __has_sse3_support_v = static_cast<int>(_ISA_) >= static_cast<int>(arch::ISA::SSE3);

template <arch::ISA _ISA_>
constexpr inline bool __has_ssse3_support_v = static_cast<int>(_ISA_) >= static_cast<int>(arch::ISA::SSSE3);

template <arch::ISA _ISA_>
constexpr inline bool __has_sse41_support_v = static_cast<int>(_ISA_) >= static_cast<int>(arch::ISA::SSE41);

template <arch::ISA _ISA_>
constexpr inline bool __has_sse42_support_v = static_cast<int>(_ISA_) >= static_cast<int>(arch::ISA::SSE42);

template <arch::ISA _ISA_>
constexpr inline bool __has_avx2_support_v = static_cast<int>(_ISA_) >= static_cast<int>(arch::ISA::AVX2);

template <arch::ISA _ISA_> 
constexpr inline bool __has_avx512f_support_v = static_cast<int>(_ISA_) >= static_cast<int>(arch::ISA::AVX512F);

template <arch::ISA _ISA_>
constexpr inline bool __has_avx512bw_support_v = static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512BW)
	|| static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512BWDQ)
	|| static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512VLBWDQ)
	|| static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512VLBW);

template <arch::ISA _ISA_>
constexpr inline bool __has_avx512dq_support_v = static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512DQ)
	|| static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512BWDQ)
	|| static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512VLBWDQ)
	|| static_cast<int>(_ISA_) == static_cast<int>(arch::ISA::AVX512VLDQ);

__RAZE_DATAPAR_NAMESPACE_END
