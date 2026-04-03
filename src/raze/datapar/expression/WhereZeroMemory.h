#pragma once

#include <src/raze/datapar/Arithmetic.h>
#include <src/raze/datapar/Bitwise.h>
#include <src/raze/datapar/expression/IsWhereMemory.h>
#include <src/raze/datapar/SimdIntegralTypesCheck.h>
#include <src/raze/datapar/SimdMaskTypeCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    class _IteratorType_,
    class _MaskType_>
class _Where_zero_memory {
    static_assert(type_traits::is_iterator_v<_IteratorType_>);
    static_assert(__is_simd_mask_v<_MaskType_>);
public:
    using mask_type     = _MaskType_;
    using value_type    = typename mask_type::element_type;
    using abi_type      = typename mask_type::abi_type;
    using datapar_type  = simd<value_type, abi_type>;
    using iterator_type = _IteratorType_;

    static constexpr auto __isa     = mask_type::__isa;
    static constexpr auto __width   = mask_type::__width;

    _Where_zero_memory(
        iterator_type       __begin,
        const mask_type&    __mask) noexcept:
            _begin(__begin),
            _mask(__mask)
    {}
    
    template <class _AlignmentPolicy_>
    raze_nodiscard raze_always_inline datapar_type __load(_AlignmentPolicy_&& __policy) const noexcept {
        return _Maskz_load<__isa, __width, value_type, typename datapar_type::vector_type>()(
            std::to_address(_begin), __data(_mask), __policy);
    }

    _Where_zero_memory(const _Where_zero_memory&) noexcept = default;
    _Where_zero_memory(_Where_zero_memory&&) noexcept = default;

    ~_Where_zero_memory() = default;

private:
    iterator_type _begin;
    mask_type _mask;
};

__RAZE_DATAPAR_NAMESPACE_END
