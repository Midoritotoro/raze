#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/mask/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/AnyOf.h>
#  include <src/raze/vx/hw/x86/mask/operations/ClearLast.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_clear_last : raze::options::strict_elementwise_callable<_Configurable_clear_last, _Options_, not_null_option> {
    template <simd_mask_type _Type_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x) const noexcept {
        return raze::options::__dispatch_call(*this, __x);
    }

    template <simd_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        constexpr auto __unsafe = _Options_::contains(not_null) && _Type_::__chunks_count() == 1;

        _Type_ __r = __x;
        __r.__for_each_chunk_any_of_reverse([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
            if constexpr (__unsafe) {
                __chunk = __clear_last<_Abi_::isa, _Value_, _Chunk::size, __unsafe>(__storage_unwrap(__chunk));
                return true;
            }
            else {
                if (_Any_of<_Abi_::isa, _Value_>()(__storage_unwrap(__chunk))) {
                    __chunk = __clear_last<_Abi_::isa, _Value_, _Chunk::size, __unsafe>(__storage_unwrap(__chunk));
                    return true;
                }
                else return false;
            }
        });
        return __r;
    }

    using callable_tag_type = _Configurable_clear_last;
};

constexpr inline auto __clear_last_impl = raze::options::functor<_Configurable_clear_last>;

__RAZE_VX_NAMESPACE_END
