__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    class _RebindType_,
    class _VectorType_,
    bool _IsBasicSimd_,
    bool _IsIntrin_>
struct __rebind_vector_element_t {
    using type = void;
};

template <
    class _RebindType_,
    class _VectorType_>
struct __rebind_vector_element_t<_RebindType_, _VectorType_, false, true> {
    using type = std::conditional_t<__is_intrin_type_v<_RebindType_> || __is_valid_simd_v<_RebindType_>, _RebindType_,
        std::conditional_t<sizeof(_VectorType_) == 64,
        type_traits::__deduce_simd_vector_type<arch::ISA::AVX512F, _RebindType_, 512>,
        std::conditional_t<sizeof(_VectorType_) == 32,
        type_traits::__deduce_simd_vector_type<arch::ISA::AVX2, _RebindType_, 256>,
        std::conditional_t<sizeof(_VectorType_) == 16,
        type_traits::__deduce_simd_vector_type<arch::ISA::SSE2, _RebindType_, 128>, void>>>>;
};

template <
    class _RebindType_,
    class _VectorType_>
struct __rebind_vector_element_t<_RebindType_, _VectorType_, true, false> {
    using type = std::conditional_t<__is_intrin_type_v<_RebindType_> || __is_valid_simd_v<_RebindType_>,
        _RebindType_, simd<_VectorType_::__isa, _RebindType_, _VectorType_::__width>>;
};

template <
    arch::ISA	_ToSimdGeneration_,
    class               _RebindType_,
    class               _VectorType_,
    bool                _IsBasicSimd_,
    bool                _IsIntrin_>
struct __rebind_vector_generation_t {
    using type = void;
};

template <
    arch::ISA	_ToSimdGeneration_,
    class               _RebindType_,
    class               _VectorType_>
struct __rebind_vector_generation_t<_ToSimdGeneration_, _RebindType_, _VectorType_, false, true> {
    using type = type_traits::__deduce_simd_vector_type<_ToSimdGeneration_, _RebindType_, __default_width<_ToSimdGeneration_>>;
};

template <
    arch::ISA	_ToSimdGeneration_,
    class               _RebindType_,
    class               _VectorType_>
struct __rebind_vector_generation_t<_ToSimdGeneration_, _RebindType_, _VectorType_, true, false> {
    using type = simd<_ToSimdGeneration_, _RebindType_, __default_width<_ToSimdGeneration_>>;
};

template <
    class _RebindType_,
    class _VectorType_>
using __rebind_vector_element_type = typename __rebind_vector_element_t<_RebindType_, _VectorType_>::type;

template <
    arch::ISA	_ToSimdGeneration_,
    class               _RebindType_,
    class               _VectorType_>
using __rebind_vector_generation_type = typename __rebind_vector_generation_t<_ToSimdGeneration_, _RebindType_, _VectorType_>::type;



template <class _VectorType_>
__simd_nodiscard_inline __unwrapped_vector_type<_VectorType_> __simd_unwrap(_VectorType_ __vector) noexcept {
    if constexpr (__is_valid_simd_v<_VectorType_>)
        return __vector.unwrap();
    else
        return __vector;
}

template <class _MaskType_, std::enable_if_t<__is_valid_simd_v<_MaskType_> ||
    __is_intrin_type_v<_MaskType_> || std::is_integral_v<_MaskType_> || 
    __is_simd_mask_v<_MaskType_> || __is_simd_index_mask_v<_MaskType_>, int>>
__simd_nodiscard_inline auto __simd_unwrap_mask(_MaskType_ __mask) noexcept {
    if constexpr (__is_simd_mask_v<_MaskType_> || __is_simd_index_mask_v<_MaskType_>)
        return __mask.unwrap();
    else if constexpr (std::is_integral_v<_MaskType_>)
        return __mask;
    else
        return __simd_unwrap(__mask);
}

template <
    class _ToType_,
    class _FromType_,
    std::enable_if_t<(__is_valid_simd_v<_ToType_> || __is_intrin_type_v<_ToType_> ||
        type_traits::__is_vector_type_supported_v<_ToType_>) &&
    (__is_valid_simd_v<_FromType_> || __is_intrin_type_v<_FromType_>), int>>
__simd_nodiscard_inline __rebind_vector_element_type<_ToType_, _FromType_> simd_cast(_FromType_ __from) noexcept {
    return __intrin_bitcast<__unwrapped_vector_type<__rebind_vector_element_type<_ToType_, _FromType_>>>(__simd_unwrap(__from));
}

template <
    arch::ISA	_ToSimdGeneration_,
    class               _FromVector_,
    std::enable_if_t<__is_valid_simd_v<_FromVector_> || __is_intrin_type_v<_FromVector_>, int>>
__simd_nodiscard_inline __rebind_vector_generation_type<_ToSimdGeneration_,
    __vector_element_type<_FromVector_>, _FromVector_> simd_cast(_FromVector_ __from) noexcept
{
    return __intrin_bitcast<__unwrapped_vector_type<__rebind_vector_generation_type<_ToSimdGeneration_,
        __vector_element_type<_FromVector_>, _FromVector_>>>(__simd_unwrap(__from));
}

template <
    arch::ISA	_ToSimdGeneration_,
    class               _ToElementType_,
    class               _FromVector_,
    std::enable_if_t<__is_valid_simd_v<_FromVector_> || __is_intrin_type_v<_FromVector_>, int>>
__simd_nodiscard_inline __rebind_vector_generation_type<_ToSimdGeneration_,
    _ToElementType_, _FromVector_> simd_cast(_FromVector_ __from) noexcept 
{
    return __intrin_bitcast<__unwrapped_vector_type<__rebind_vector_generation_type<_ToSimdGeneration_,
        _ToElementType_, _FromVector_>>>(__simd_unwrap(__from));
}

__RAZE_DATAPAR_NAMESPACE_END
