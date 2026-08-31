#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/Ternarylogic.h>
#  include <src/raze/vx/hw/x86/mask/operations/MaskTernarylogic.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_ternarylogic_t: options::conditional_callable<configurable_ternarylogic_t, Options> {
    template <simd_or_mask_type T, u8 Op>
    raze_nodiscard raze_always_inline T operator()(const T& x, 
        const T& y, const T& z, std::integral_constant<u8, Op> op) const noexcept
    {
        return options::dispatch_call(*this, x, y, z, op);
    }

    template <simd_or_mask_type T, u8 Op>
    static raze_always_inline auto deferred_call(auto opts, const T& x, 
        const T& y, const T& z, std::integral_constant<u8, Op> op) noexcept
    {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename T::value_type;
        using Abi = typename T::abi_type;

        T r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            if constexpr (simd_mask_type<T>) chunk = ternarylogic_<Abi::isa, T>(ustorage(chunk), ustorage<Args>(args)...);
            else chunk = mask_ternarylogic_<Abi::isa, T>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, y.__storage().storage(), z.__storage().storage(), 
                    op, condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, y.__storage().storage(), z.__storage().storage(), op, condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op, y.__storage().storage(), z.__storage().storage(), op);
        }

        return r;
    }
};

constexpr inline auto ternarylogic = options::functor<configurable_ternarylogic_t>;

__RAZE_VX_NAMESPACE_END
