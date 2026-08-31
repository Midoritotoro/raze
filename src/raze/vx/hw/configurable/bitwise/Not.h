#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/bitwise/BitNot.h>
#  include <src/raze/vx/hw/x86/mask/operations/MaskNot.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_not_t: options::conditional_callable<configurable_not_t, Options> {
    template <simd_or_mask_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x) const noexcept {
        return options::dispatch_call(*this, x);
    }

    template <simd_or_mask_type T>
    static raze_always_inline auto deferred_call(auto opts, const T& x) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename T::value_type;
        using Abi = typename T::abi_type;

        T r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            if constexpr (simd_mask_type<T>) chunk = mask_not_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
            else chunk = bit_not_<Abi::isa, Value>(ustorage(chunk), ustorage<Args>(args)...);
        };

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op);
        }

        return r;
    }
};

constexpr inline auto bit_not = options::functor<configurable_not_t>;

__RAZE_VX_NAMESPACE_END
