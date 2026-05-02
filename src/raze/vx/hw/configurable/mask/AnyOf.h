#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/AnyOf.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_any_of: raze::options::strict_elementwise_callable<_Configurable_any_of, _Options_> {
    template <simd_mask_type _Type_>
    raze_nodiscard raze_always_inline bool operator()(const _Type_& __x) const noexcept {
        return raze::options::__dispatch_call(*this, __x);
    }

    template <simd_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        auto __chunk_op = [&] <class _Chunk, class ... _Args> (const _Chunk& __chunk, _Args&&... __args) raze_always_inline_lambda {
            return _Any_of<_Abi_::isa, _Value_>()(__chunk.data(), std::forward<_Args>(__args)...);
        };

        if constexpr (!options::concepts::same_as<_Mask_, options::unknown_key>) {
            auto __condition = __options[raze::options::condition_key];
            const auto __mask = __condition.mask(raze::options::as<typename _Mask_::condition_type>{});

            if constexpr (_Mask_::has_alternative)
                return __x.__for_each_chunk_any_of(__chunk_op, __data(__mask), __data(__condition.alternative()));
            else
                return __x.__for_each_chunk_any_of(__chunk_op, __data(__mask));
        }
        else {
            return __x.__for_each_chunk_any_of(__chunk_op);
        }
    }

    using callable_tag_type = _Configurable_any_of;
};

__RAZE_VX_NAMESPACE_END
