#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/mask/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/FindNextSet.h>
#  include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_find_next_set : raze::options::strict_elementwise_callable<_Configurable_find_next_set, _Options_, not_null_option> {
    template <simd_mask_type _Type_>
    raze_nodiscard raze_always_inline i32 operator()(const _Type_& __x, i32 __from) const noexcept {
        return raze::options::__dispatch_call(*this, __x, __from);
    }

    template <simd_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, i32 __from) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        i32 __index = 0;
        constexpr auto __unsafe = _Options_::contains(not_null) && _Type_::__chunks_count() == 1;

        i32 __shift = __from < 0 ? 0u : u32(__from + 1);

        auto __chunk_op = [&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
            if (__shift >= i32(_Chunk::size)) {
                __index += _Chunk::size;
                __shift -= _Chunk::size;
                return true;
            }

            auto __r = _Find_next_set<_Abi_::isa, _Chunk::size, _Value_, __unsafe>()(__storage_unwrap(__chunk), __shift);
            __index += __r;

            if (__r != _Chunk::size) return false;

            __shift = 0;
            return true;
        };

        __x.__for_each_chunk_all_of(__chunk_op);
        return __index;
    }

    using callable_tag_type = _Configurable_find_next_set;
};

__RAZE_VX_NAMESPACE_END
