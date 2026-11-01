#pragma once 

#include <raze/options/Options.h>
#include <src/raze/traits/FunctionPass.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/reduce/HorizontalSum.h>
#include <src/raze/vx/hw/configurable/shuffle/SwapAdjacent.h>
#include <src/raze/vx/hw/configurable/merge/Select.h>
#include <src/raze/traits/FunctionPass.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/reduce/Fold.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

struct uniform_mode {};
constexpr inline auto uniform = raze::options::flag(uniform_mode{});
struct uniform_option : raze::options::exact_option<uniform> {};

//struct mirror_mode {};
//constexpr inline auto mirror = raze::options::flag(mirror_mode{});
//struct mirror_option : raze::options::exact_option<mirror> {};

template <class Options>
struct configurable_fold_t : options::conditional_callable<configurable_fold_t, Options, uniform_option> {
    template <simd_type V, class F>
    raze_nodiscard raze_always_inline V operator()(const V& x, F f) const noexcept
        requires(Options::contains(uniform))
    {
        return options::dispatch_call(*this, x, traits::fwd_fn(f));
    }

    template <simd_type V, class F>
    raze_nodiscard raze_always_inline typename V::value_type operator()(const V& x, F f) const noexcept
        requires(!Options::contains(uniform))
    {
        return options::dispatch_call(*this, x, traits::fwd_fn(f));
    }

    template <simd_type V, class F>
    static raze_always_inline auto deferred_call(auto opts, const V& x, F f) noexcept
        requires(std::is_same_v<traits::function_unwrapped<std::remove_cvref_t<F>>, std::remove_cvref_t<decltype(add)>>)
    {
        return hsum[opts](x);
    }

    template <simd_type V, class F>
    static raze_always_inline auto deferred_call(auto opts, V x, F f) noexcept
        requires(!std::is_same_v<traits::function_unwrapped<std::remove_cvref_t<F>>, std::remove_cvref_t<decltype(add)>>)
    {
        using Value = typename V::value_type;

        if constexpr (V::size() == 1) {
            if constexpr (Options::contains(uniform)) return select[opts](x);
            else return Value(select[opts](x)[std::integral_constant<sizetype, 0>{}]);
        }
        else {
            constexpr auto depth = std::bit_width(sizetype(V::size())) - 1;

            const auto r = [&] <sizetype ... Indices> (std::integer_sequence<sizetype, Indices...>) raze_always_inline_lambda {
                x = select[opts](x);
                ((x = f(x, swap_adjacent(x, std::integral_constant<sizetype, 1ull << Indices>{}))), ...);
                return x;
            } (std::make_integer_sequence<sizetype, depth>{});

            if constexpr (Options::contains(uniform)) return r;
            else return r[std::integral_constant<sizetype, 0>{}];
        }
    }
};

/**
 * @ingroup raze_vx_reduce
 * @memberof raze::vx
 * @hideinitializer
 * @brief Reduces the elements of a SIMD vector to a single scalar or a uniform vector.
 *
 * Applies a binary operation `f` to all lanes of `x` in a tree-like reduction pattern.
 * By default, returns a scalar. If the `uniform` option is provided, returns a vector
 * with the reduced result broadcast to all lanes.  
 *
 * ### Declarations
 * ```cpp
 * // (1) Uniform vector return
 * template <simd_type V, class F>
 * V operator()(const V& x, F f) const noexcept;
 *
 * // (2) Scalar return
 * template <simd_type V, class F>
 * typename V::value_type operator()(const V& x, F f) const noexcept;
 * ```
 *
 * ### Template Parameters
 * - `V` — The source SIMD type. Defines the lane count, element type, and ABI.
 * - `F` — A binary callable object (e.g., `raze::vx::add`, `std::plus`).
 *
 * ### Parameters
 * - `x` — The input vector to reduce.
 * - `f` — The binary reduction function.
 *
 * ### Options
 * - `raze::vx::uniform` — Returns the result as a SIMD vector with the scalar broadcast to all lanes,
 *   instead of a single scalar value.
 *
 * ### Mask semantics
 * Supports the standard forms described in @ref mask_semantics
 * Masking is applied to the input vector *before* the reduction.  
 *
 * ### Return value
 * A scalar of type `V::value_type` (default), or a vector of type `V` if the `uniform` option is used.
*/
constexpr inline auto fold = options::functor<configurable_fold_t>;

__RAZE_VX_NAMESPACE_END
