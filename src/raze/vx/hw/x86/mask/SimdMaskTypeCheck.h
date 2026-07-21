#pragma once 

#include <src/raze/type_traits/TypeCheck.h>
#include <src/raze/type_traits/IsVirtualBaseOf.h>

__RAZE_VX_NAMESPACE_BEGIN

template <u64 _N_>
using __mmask_for_elements_helper = std::conditional_t<_N_ <= 8, __mmask8,
    std::conditional_t<_N_ <= 16, __mmask16,
    std::conditional_t<_N_ <= 32, __mmask32,
    std::conditional_t<_N_ <= 64, __mmask64, void>>>>;

template <u64 _N_>
using __mmask_for_elements_t = __mmask_for_elements_helper<_N_>;

template <arch::ISA _ISA_>
constexpr auto __default_width = arch::__is_xmm_v<_ISA_> ? 128 : arch::__is_ymm_v<_ISA_> ? 256 : arch::__is_zmm_v<_ISA_> ? 512 : -1;

template <arch::ISA _ISA_>
constexpr auto __vector_default_size = __default_width<_ISA_>;

template <class _Type_, class _Abi_>
class simd_mask;

template <class _SimdMask_, class = void>
struct __is_simd_mask :
	std::false_type
{};

template <class _SimdMask_>
struct __is_simd_mask<
	_SimdMask_,
    std::void_t<simd_mask<typename _SimdMask_::element_type,
                typename _SimdMask_::abi_type>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            simd_mask<typename _SimdMask_::element_type,
                typename _SimdMask_::abi_type>,
            _SimdMask_> ||
        std::is_same_v<simd_mask<typename _SimdMask_::element_type,
				typename _SimdMask_::abi_type>,
            _SimdMask_>> 
{};

template <class _SimdMask_>
constexpr bool __is_simd_mask_v = __is_simd_mask<std::remove_cvref_t<_SimdMask_>>::value;

__RAZE_VX_NAMESPACE_END