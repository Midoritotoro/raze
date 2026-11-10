#pragma once

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/Storage.h>
#include <src/raze/vx/storage/Node.h>
#include <src/raze/vx/storage/Size.h>
#include <tuple>
#include <utility>
#include <type_traits>
#include <bit>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Convert.h>
#endif // defined(raze_processor_x86)

__RAZE_VX_NAMESPACE_BEGIN

struct ignore_unsupported_mode {};
inline constexpr auto ignore_unsupported = raze::options::flag(ignore_unsupported_mode{});
struct ignore_unsupported_option : raze::options::exact_option<ignore_unsupported> {};

struct saturated_mode {};
inline constexpr auto saturated = raze::options::flag(saturated_mode{});
struct saturated_option : raze::options::exact_option<saturated> {};

template <class R>
struct reg_count { static constexpr sizetype value = 1; };

template <class... Ts>
struct reg_count<std::tuple<Ts...>> { static constexpr sizetype value = sizeof...(Ts); };

template <class H, class T>
struct reg_count<simd_tuple_node<H, T>> {
    static constexpr sizetype value = 1 + reg_count<std::remove_cvref_t<T>>::value;
};

template <>
struct reg_count<simd_tuple_nil> { static constexpr sizetype value = 0; };

template <class R>
inline constexpr sizetype reg_count_v = reg_count<std::remove_cvref_t<R>>::value;

template <sizetype Idx, class V>
raze_always_inline auto get_src_chunk(const V& x) noexcept {
    using RawV = std::remove_cvref_t<V>;
    auto& chunk = const_cast<RawV&>(x).template __get<Idx>();
    return ustorage(chunk);
}

template <sizetype Idx, class V, class To>
consteval sizetype get_chunk_dst_offset() {
    if constexpr (Idx == 0) return 0;
    else {
        using Value = typename V::value_type;
        using Abi   = typename V::abi_type;

        return [] <sizetype... Is> (std::index_sequence<Is...>) {
            return (reg_count_v<deduce_convert_return_type<Abi::isa, Value, To,
                decltype(get_src_chunk<Is>(std::declval<const V&>()))>> +...);
        }(std::make_index_sequence<Idx>{});
    }
}

template <class V, class To>
consteval sizetype get_total_dst_regs() {
    return get_chunk_dst_offset<V::__chunks_count(), V, To>();
}

template <sizetype Idx, class V, class Result>
consteval bool chunk_size_matches() {
    using Value = typename V::value_type;
    using To = typename Result::value_type;
    using Abi = typename V::abi_type;

    constexpr sizetype offset = get_chunk_dst_offset<Idx, V, To>();
    using InChunk = decltype(get_src_chunk<Idx>(std::declval<const V&>()));
    using OutRegs = deduce_convert_return_type<Abi::isa, Value, To, InChunk>;

    auto slot_size = [] <sizetype K> () constexpr {
        return sizeof(std::remove_cvref_t<decltype(ustorage(std::declval<Result&>().template __get<K>()))>);
    };

    if constexpr (vx::is_std_tuple_v<OutRegs>) {
        return [=] <sizetype... Ks> (std::index_sequence<Ks...>) {
            return ((sizeof(std::tuple_element_t<Ks, OutRegs>) ==
                slot_size.template operator() < offset + Ks > ()) && ...);
        }(std::make_index_sequence<std::tuple_size_v<OutRegs>>{});
    }
    else if constexpr (is_simd_tuple<OutRegs>::value) {
        return [=] <sizetype... Ks> (std::index_sequence<Ks...>) {
            return ((sizeof(simd_tuple_element_t<Ks, OutRegs>) ==
                slot_size.template operator() < offset + Ks > ()) && ...);
        }(std::make_index_sequence<simd_tuple_size<OutRegs>::value>{});
    }
    else return sizeof(OutRegs) == slot_size.template operator() < offset > ();
}

template <class V, class Result>
consteval bool check_all_chunk_sizes_match() {
    using To = typename Result::value_type;

    if constexpr (Result::__chunks_count() != get_total_dst_regs<V, To>()) return false;
    else return [] <sizetype... Is> (std::index_sequence<Is...>) {
        return (chunk_size_matches<Is, V, Result>() && ...);
    }(std::make_index_sequence<V::__chunks_count()>{});
}

template <bool Saturated, sizetype I, class V, class T, class Dst>
raze_always_inline void convert_chunk_into(Dst& dst, const V& x) noexcept {
    using Value = typename V::value_type;
    using Abi = typename V::abi_type;
    assign_regs<get_chunk_dst_offset<I, V, T>()>(dst, convert_<Abi::isa, Saturated, Value, T>(get_src_chunk<I>(x)));
}

template <traits::trivially_copyable T>
struct configurable_convert_t {
    template <class Options>
    struct convert : options::conditional_callable<convert, Options, saturated_option, ignore_unsupported_option> {
        template <simd_type V>
        raze_nodiscard raze_always_inline auto operator()(const V& x) const noexcept {
            if constexpr (Options::contains(ignore_unsupported) && !arithmetic_type<T>) return x;
            else return options::dispatch_call(*this, x);
        }

        template <simd_type V>
        static raze_always_inline auto deferred_call(auto, const V& x) noexcept {
            using Abi = typename V::abi_type;
            using ResultSimd = simd<T, Abi>;

            ResultSimd res(uninitialized);

            constexpr sizetype num_src_chunks = V::__chunks_count();
            constexpr auto can_fast_convert = check_all_chunk_sizes_match<V, ResultSimd>();
            constexpr auto is_saturated = Options::contains(saturated);

            if constexpr (can_fast_convert) {
                [&] <sizetype... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
                    (convert_chunk_into<is_saturated, Is, V, T>(res, x), ...);
                }(std::make_index_sequence<num_src_chunks>{});
            }
            else {
                for (sizetype i = 0; i < Abi::size; ++i) {
                    if constexpr (is_saturated) res[i] = math::saturating_cast<T>(x[i]);
                    else res[i] = static_cast<T>(x[i]);
                }
            }

            return res;
        }
    };
};

/**
 * @ingroup raze_vx_arith
 * @memberof raze::vx
 * @hideinitializer
 * @brief Converts every lane of a SIMD vector to a different arithmetic type, with conditional pass-through for unsupported types.
 *
 * Returns a SIMD vector of type `simd<T, V::abi_type>` where each lane is converted from the
 * source element type to the destination type `T`.
 *
 * By default, standard C++ `static_cast` semantics are used (truncation). If the
 * `raze::vx::saturated` option is attached, conversions are clamped to the destination
 * type's limits.
 *
 * If the `raze::vx::ignore_unsupported` option is attached and the types are **not** arithmetic
 * (i.e., unsupported by standard conversion), the function acts as an identity function,
 * returning the input vector `x` unchanged (bit-wise).
 * However, if `[raze::vx::ignore_unsupported]` is provided but the types *are* arithmetic, the normal
 * conversion (or saturated conversion) is still performed. This allows generic algorithms
 * to use a unified code path for both arithmetic types and raw `trivially_copyable` types.
 *
 * ### Saturation semantics
 * When `[raze::vx::saturated]` is used (and types are arithmetic), the conversion follows these rules:
 * - **Float to Integer**: `NaN` becomes `0`. Values out of bounds are clamped to `min`/`max`.
 * - **Float to Float (narrowing)**: `NaN` remains `NaN`. Out-of-bounds values are clamped.
 * - **Integer to Integer (narrowing)**: Out-of-bounds values are clamped to `min`/`max`.
 * - **Signed to Unsigned**: Negative values become `0`. Values exceeding `max` are clamped.
 * - **Unsigned to Signed**: Values exceeding `max` are clamped.
 *
 * ### Declarations
 * ```cpp
 * template <typename T>
 * constexpr inline *unspecified* convert;
 * ```
 *
 * ### Template Parameters
 * - `T` — The destination type (e.g., `int8_t`, `float`, or a `trivially_copyable` struct when `[raze::vx::ignore_unsupported]` is used).
 *
 * ### Parameters
 * - `x` — The source SIMD vector.
 *
 * ### Mask semantics
 * **Not supported.**
 *
 * ### Return value
 * - For arithmetic types: A vector of type `simd<T, V::abi_type>` containing the converted lanes.
 * - For unsupported types with `[raze::vx::ignore_unsupported]`: The original source vector `x` returned unchanged.
*/
template <arithmetic_type T>
constexpr inline auto convert = options::functor<configurable_convert_t<T>::template convert>;

/**
 * @ingroup raze_vx_concepts
 * @memberof raze::vx
 * @hideinitializer
 * @brief Heuristic hint: will `convert<To>` on this SIMD type emit native SIMD?
 *
 * Detects whether converting every lane of a SIMD value of type `V` to element
 * type `To` is **likely** to be lowered to native SIMD instructions by the
 * current compiler and ABI, or will degrade into per-lane scalar code.
 *
 * ### What it actually checks
 *
 * The check is deliberately conservative and platform-aware:
 *
 * - **MSVC**: returns `true` only when `V` is guaranteed to occupy at least ones
 *   full hardware register (`sizeof(V) >= 16`) **and** the destination type is
 *   at least register-wide (`V::size() * sizeof(To) >= 16`). This is the
 *   minimum bar for MSVC to avoid `simd` being represented as a *tuple of
 *   scalars*, which forces the codegen into per-lane loops with mask-blends
 *   and register spills — the worst-case pathological output.
 * - **GCC / Clang / ICPX**: returns `true` unconditionally. These compilers
 *   have mature SIMD backends and a general-purpose vectorizer, so native
 *   SIMD lowering is the overwhelmingly common case — even when the check
 *   cannot prove it in advance.
 *
 * ### Caveats
 *
 * - **"Fast" is subjective.** This concept does **not** measure performance.
 *   It is a *codegen heuristic*, not a benchmark. A return value of `true`
 *   only means "the compiler is likely to emit SIMD instructions", not
 *   "this code path will be faster than scalar". 
 * - **`true` ≠ SIMD.** Even on GCC/Clang/ICPX, some conversions on exotic
 *   types or unusual register widths may still fall back to scalar
 *   per-element loops. The concept is a *hint to gate a manual fast-path*,
 *   not a guarantee that the fast-path is engaged.
 * - **`false` ≠ correctness issue.** Returning `false` never means the
 *   conversion is wrong. It means "the compiler is known to take a
 *   slow path", and the caller should let the algorithm use its own
 *   scalar fallback instead of forcing a half-vectorized middle ground.
 *
 * ### Why bother
 *
 * On MSVC, gating a SIMD fast-path with this concept avoids the worst
 * codegen (nested per-lane loops and spills
 * inside the inner vector loop). On GCC/Clang/ICPX, the concept is almost
 * always `true`, so it costs nothing in practice and keeps the code
 * portable across compilers.
 *
 * ### Example
 * ```cpp
 * // Or, in user code, decide whether to engage vectorization:
 * if constexpr (vx::supports_fast_conversion<simd<float, fixed_abi<4>>, char>) {
 *     // On GCC/Clang/ICPX:      true  → SIMD path
 *     // On MSVC (simd<char,8>): false → scalar fallback avoids spills
 * }
 * ```
*/
template <class V, class To>
concept supports_fast_conversion = simd_type<V> && arithmetic_type<To> &&
    (!compatibility::is_msvc_only || (((V::size() * sizeof(typename V::value_type)) >= 16) && ((V::size() * sizeof(To)) >= 16)));

__RAZE_VX_NAMESPACE_END