#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/IsContiguous.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_is_contiguous: raze::options::strict_elementwise_callable<_Configurable_is_contiguous, _Options_> {
    template <simd_mask_type _Type_>
    raze_nodiscard raze_always_inline bool operator()(const _Type_& __x, i32 __n, i32 __k) const noexcept {
        raze_debug_assert(__n < _Type_::size() && __k < _Type_::size() && __n <= __k);
        return raze::options::__dispatch_call(*this, __x, __n, __k);
    }

    // 16 8
    // 17 20
    //  
    template <simd_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, i32 __n, i32 __k) noexcept {
        static_assert(raze::options::concepts::same_as<raze::options::fetch_t<raze::options::condition_key, _Options_>,
            raze::options::unknown_key>, "is_contiguous does not support conditional masks via []. ");

        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        if constexpr (_Type_::__is_native() || _Type_::size() == 1) {
            return __x.__for_each_chunk_any_of([&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
                return _Is_contiguous<_Abi_::isa, _Chunk::size, _Value_>()(__storage_unwrap(__chunk), __n, __k);
            });
        }
        else {
            i32 base = 0;

            return __x.__for_each_chunk_all_of(
                [&]<class _Chunk>(const _Chunk & chunk) {

                constexpr i32 C = _Chunk::size;

                const i32 begin = (std::max)(__n - base, 0);
                const i32 end = (std::min)(__k - base, C);

                base += C;

                if (begin >= end)
                    return true;

                return _Is_contiguous<_Abi_::isa, C, _Value_>()(
                    __storage_unwrap(chunk), begin,  end);
            });
        }
    }

    using callable_tag_type = _Configurable_is_contiguous;
};

__RAZE_VX_NAMESPACE_END
