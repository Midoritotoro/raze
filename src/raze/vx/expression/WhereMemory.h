#pragma once

#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/vx/hw/Bitwise.h>
#include <src/raze/vx/expression/IsWhereMemory.h>
#include <src/raze/vx/hw/x86/mask/SimdMaskTypeCheck.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    class _IteratorType_,
    class _DataparType_,
    class _MaskType_>
class _Where_memory {
    static_assert(type_traits::is_iterator_v<_IteratorType_>);
    static_assert(__is_simd_mask_v<_MaskType_>);
    static_assert(__is_valid_simd_v<_DataparType_>);
public:
    using datapar_type  = _DataparType_;
    using mask_type     = _MaskType_;
    using value_type    = typename mask_type::element_type;
    using abi_type      = typename mask_type::abi_type;
    using iterator_type = _IteratorType_;

    static constexpr auto __isa     = mask_type::__isa;
    static constexpr auto __width   = mask_type::__width;

    _Where_memory(
        iterator_type       __begin,
        const datapar_type& __source,
        const mask_type&    __mask) noexcept:
            _begin(__begin),
            _source(__source),
            _mask(__mask)
    {}
    
    template <class _AlignmentPolicy_>
    raze_nodiscard raze_always_inline datapar_type __load(_AlignmentPolicy_&& __policy) const noexcept {
        return _Mask_load<__isa, __width, value_type>()(std::to_address(_begin),
            __data(_mask), __data(_source), __policy);
    }

    _Where_memory(const _Where_memory&) noexcept = default;
    _Where_memory(_Where_memory&&) noexcept = default;

    ~_Where_memory() = default;

private:
    iterator_type _begin;
    datapar_type _source;
    mask_type _mask;
};

__RAZE_VX_NAMESPACE_END

