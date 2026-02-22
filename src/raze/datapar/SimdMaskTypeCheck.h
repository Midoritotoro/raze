#pragma once 

#include <src/raze/type_traits/TypeCheck.h>
#include <src/raze/type_traits/IsVirtualBaseOf.h>

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <arch::ISA _ISA_>
constexpr auto __default_width =
arch::__is_xmm_v<_ISA_> ? 128 :
    arch::__is_ymm_v<_ISA_> ? 256 :
    arch::__is_zmm_v<_ISA_> ? 512 : -1;


template <arch::ISA _ISA_>
constexpr auto __vector_default_size = __default_width<_ISA_>;

template <
    arch::ISA	_ISA_,
    class       _Type_,
    uint32      _SimdWidth_ = __default_width<_ISA_>>
class simd_index_mask;

template <
	class _SimdMask_, 
	class = void>
struct __is_simd_index_mask :
	std::false_type
{};

template <class _SimdMask_>
struct __is_simd_index_mask<
	_SimdMask_,
    std::void_t<simd_index_mask<
        _SimdMask_::__isa,
        typename _SimdMask_::element_type,
        _SimdMask_::__width>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            simd_index_mask<
				_SimdMask_::__isa,
                typename _SimdMask_::element_type,
                _SimdMask_::__width>,
            _SimdMask_> ||
        std::is_same_v<
            simd_index_mask<
				_SimdMask_::__isa,
				typename _SimdMask_::element_type,
				_SimdMask_::__width>,
            _SimdMask_>> 
{};

template <class _SimdMask_>
constexpr bool __is_simd_index_mask_v = __is_simd_index_mask<_SimdMask_>::value;

template <
	arch::ISA	_ISA_,
	class       _Type_,
	uint32      _SimdWidth_ = __default_width<_ISA_>>
class simd_mask;

template <
	class _SimdMask_, 
	class = void>
struct __is_simd_mask :
	std::false_type
{};

template <class _SimdMask_>
struct __is_simd_mask<
	_SimdMask_,
    std::void_t<simd_mask<_SimdMask_::__isa,
                typename _SimdMask_::element_type,
                _SimdMask_::__width>>>
    : std::bool_constant<
        type_traits::is_virtual_base_of_v<
            simd_mask<_SimdMask_::__isa,
                typename _SimdMask_::element_type,
                _SimdMask_::__width>,
            _SimdMask_> ||
        std::is_same_v<
            simd_mask<_SimdMask_::__isa,
				typename _SimdMask_::element_type,
				_SimdMask_::__width>,
            _SimdMask_>> 
{};

template <class _SimdMask_>
constexpr bool __is_simd_mask_v = __is_simd_mask<_SimdMask_>::value;

__RAZE_DATAPAR_NAMESPACE_END