#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Storage.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Fnms.h>
#endif // defined(raze_processor_x86)


__RAZE_MATH_NAMESPACE_BEGIN

template <class Options>
struct configurable_fnms_t: options::conditional_callable<configurable_fnms_t, Options> {
    template <class A, class B, class C>
        requires (vx::simd_type<A> || vx::simd_type<B> || vx::simd_type<C>)
    raze_nodiscard raze_always_inline std::conditional_t<vx::simd_type<A>, std::remove_cvref_t<A>,
        std::conditional_t<vx::simd_type<B>, std::remove_cvref_t<B>,
        std::remove_cvref_t<C>>> operator()(const A& x, const B& y, const C& z) const noexcept {
        using V = std::conditional_t<vx::simd_type<A>, std::remove_cvref_t<A>,
            std::conditional_t<vx::simd_type<B>, std::remove_cvref_t<B>,
            std::remove_cvref_t<C>>>;

        return options::dispatch_call(*this, V(x), V(y), V(z));
    }

    template <vx::arithmetic_type T>
    raze_nodiscard raze_always_inline T operator()(const T& x, const T& y, const T& z) const noexcept {
        return options::dispatch_call(*this, x, y, z);
    }

    template <vx::arithmetic_type T>
    static raze_always_inline auto deferred_call(auto opts, const T& x, const T& y, const T& z) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;

        if constexpr (!std::same_as<Mask, options::unknown_key>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                return vx::_Fnms<arch::ISA::SSE2, T>()(x, y, z, condition.mask(), condition.alternative());
            else
                return vx::_Fnms<arch::ISA::SSE2, T>()(x, y, z, condition.mask());
        }
        else return vx::_Fnms<arch::ISA::SSE2, T>()(x, y, z);
    }

    template <vx::simd_type V>
    static raze_always_inline auto deferred_call(auto opts, const V& x, const V& y, const V& z) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename V::value_type;
        using Abi = typename V::abi_type;

        _Type_ r = x;

        auto chunk_op = [&] <class Chunk, class ... Args> (Chunk& chunk, Args&& ... args) raze_always_inline_lambda {
            chunk = vx::_Fnms<Abi::isa, Value>()(vx::__storage_unwrap(__chunk), vx::__storage_unwrap<Args>(args)...);
        };

        if constexpr (!std::same_as<Mask, options::unknown_key>) {
            auto condition = opts[options::condition_key];

            if constexpr (Mask::has_alternative)
                r.__for_each_chunk(chunk_op, __y.__storage().storage(), __z.__storage().storage(), 
                    condition.mask().__storage().storage(), condition.alternative().__storage().storage());
            else
                r.__for_each_chunk(chunk_op, __y.__storage().storage(), __z.__storage().storage(),
                    condition.mask().__storage().storage());
        }
        else {
            r.__for_each_chunk(chunk_op, __y.__storage().storage(), __z.__storage().storage());
        }

        return r;
    }
};

constexpr inline auto fnms = raze::options::functor<configurable_fnms_t>;

__RAZE_MATH_NAMESPACE_END

