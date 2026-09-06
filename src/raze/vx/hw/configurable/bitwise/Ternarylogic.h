#pragma once 

#include <raze/options/Options.h>

#if defined(raze_processor_x86)
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

        if constexpr (options::complete_mask<Mask>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk([&] <class Chunk> (Chunk& chunk, auto ch2, auto ch3, auto cond, auto src) raze_always_inline_lambda {
                    if constexpr (simd_type<T>) chunk = raze::vx::ternarylogic_<Abi::isa, Value>(ustorage(chunk), ustorage(ch2),
                        ustorage(ch3), std::integral_constant<u8, Op>{}, ustorage(cond), ustorage(src));
                    else chunk = raze::vx::mask_ternarylogic_<Abi::isa, Value>(ustorage(chunk), ustorage(ch2),
                        ustorage(ch3), std::integral_constant<u8, Op>{}, ustorage(cond), ustorage(src));
                }, y.__storage().storage(), z.__storage().storage(), condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk([&] <class Chunk> (Chunk& chunk, auto ch2, auto ch3, auto cond) raze_always_inline_lambda {
                    if constexpr (simd_type<T>) chunk = raze::vx::ternarylogic_<Abi::isa, Value>(ustorage(chunk), ustorage(ch2),
                        ustorage(ch3), std::integral_constant<u8, Op>{}, ustorage(cond));
                    else chunk = raze::vx::mask_ternarylogic_<Abi::isa, Value>(ustorage(chunk), ustorage(ch2),
                        ustorage(ch3), std::integral_constant<u8, Op>{}, ustorage(cond));
                }, y.__storage().storage(), z.__storage().storage(), condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk([&] <class Chunk> (Chunk& chunk, auto ch2, auto ch3) raze_always_inline_lambda {
                if constexpr (simd_type<T>) chunk = raze::vx::ternarylogic_<Abi::isa, Value>(ustorage(chunk), ustorage(ch2), ustorage(ch3), std::integral_constant<u8, Op>{});
                else chunk = raze::vx::mask_ternarylogic_<Abi::isa, Value>(ustorage(chunk), ustorage(ch2), ustorage(ch3), std::integral_constant<u8, Op>{});
            }, y.__storage().storage(), z.__storage().storage());
        }

        return r;
    }
};

constexpr inline auto ternarylogic = options::functor<configurable_ternarylogic_t>;

__RAZE_VX_NAMESPACE_END
