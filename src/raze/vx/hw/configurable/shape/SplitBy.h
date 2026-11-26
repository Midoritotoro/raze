#pragma once

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype Chunks, simd_type V, sizetype Index>
constexpr sizetype split_by_chunk_size_v =
    (abi_t<V>::size / Chunks) + (Index < (abi_t<V>::size % Chunks) ? 1 : 0);

template <sizetype Chunks, simd_type V, sizetype Index>
constexpr sizetype split_by_chunk_offset_v =
    Index * (abi_t<V>::size / Chunks) + (Index < (abi_t<V>::size % Chunks) ? Index : (abi_t<V>::size % Chunks));

template <sizetype Chunks, simd_type V, sizetype Index>
using split_by_simd_t = simd<
    typename V::value_type, resize_abi_t<abi_t<V>, split_by_chunk_size_v<Chunks, V, Index>>
>;

template <sizetype Chunks, simd_type V, sizetype... Indices>
using split_by_type = std::tuple<split_by_simd_t<Chunks, V, Indices>...>;

template <sizetype ChunkSize, simd_type V, class Sequence>
struct split_by_return_impl;

template <sizetype ChunkSize, simd_type V, sizetype... Indices>
struct split_by_return_impl<ChunkSize, V, std::integer_sequence<sizetype, Indices...>> {
    using type = split_by_type<ChunkSize, V, Indices...>;
};

template <sizetype Chunks, simd_type V>
using split_by_return_type = typename split_by_return_impl<
    Chunks, V, std::make_integer_sequence<sizetype, Chunks>>::type;

template <sizetype Chunks>
struct configurable_split_by_t {
    template <class Options>
    struct impl : options::conditional_callable<impl, Options> {
        template <simd_type V> requires(Chunks > 0 && Chunks <= V::size())
        raze_nodiscard raze_always_inline split_by_return_type<Chunks, V> operator()(const V& x) const noexcept {
            return options::dispatch_call(*this, x);
        }

        template <sizetype ChunksCount, simd_type V>
        static consteval bool can_direct_chunk_split() noexcept {
            if constexpr (ChunksCount != V::__chunks_count()) {
                return false;
            }
            else {
                return [] <sizetype... I> (std::index_sequence<I...>) {
                    return ((split_by_simd_t<ChunksCount, V, I>::__chunks_count() == 1 &&
                        std::same_as<
                            std::remove_cvref_t<decltype(ustorage(std::declval<const V&>().template __get<I>()))>,
                            std::remove_cvref_t<decltype(ustorage(std::declval<split_by_simd_t<ChunksCount, V, I>&>().template __get<0>()))>
                        >) && ...);
                }(std::make_index_sequence<ChunksCount>{});
            }
        }

        template <sizetype ChunksCount, simd_type V>
        static raze_always_inline auto impl_split(const V& x) noexcept {
            constexpr sizetype count = ChunksCount;

            if constexpr (can_direct_chunk_split<ChunksCount, V>()) {
                const auto get_simd = [&] <class Vec, sizetype Idx> (Vec, std::integral_constant<sizetype, Idx>) raze_always_inline_lambda {
                    Vec vec(uninitialized);
                    vec.template __get<0>() = ustorage(x.template __get<Idx>());
                    return vec;
                };

                return [&] <sizetype... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
                    return std::tuple{ get_simd(split_by_simd_t<ChunksCount, V, I>{}, std::integral_constant<sizetype, I>{})... };
                }(std::make_integer_sequence<sizetype, count>{});
            }
            else {
                alignas(64) typename V::value_type buffer[V::size()];
                vx::store[vx::aligned](buffer, x);

                return [&] <sizetype... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
                    return std::tuple{ vx::load<split_by_simd_t<ChunksCount, V, I>>(buffer + split_by_chunk_offset_v<ChunksCount, V, I>)... };
                }(std::make_integer_sequence<sizetype, count>{});
            }
        }

        template <simd_type V>
        static raze_always_inline auto deferred_call(auto, const V& x) noexcept  {
            return impl_split<Chunks>(x);
        }
    };
};

/**
 * @ingroup raze_vx_shape
 * @memberof raze::vx
 * @hideinitializer
 * @brief Splits a SIMD vector into a tuple of smaller SIMD vectors.
 *
 * Divides the input vector `x` into `Chunks` contiguous sub-vectors. The sizes of the
 * resulting vectors are as balanced as possible.  
 *
 * ### Declarations
 * ```cpp
 * template <sizetype Chunks>
 * constexpr inline *unspecified* split_by;
 * ```
 *
 * ### Template Parameters
 * - `Chunks` — The number of parts to split the vector into. Must be greater than `0`.
 *
 * ### Parameters
 * - `x` — The input vector to split. The type `V` is deduced.
 *
 * ### Constraints
 * - `Chunks > 0`
 *
 * ### Mask semantics
 * **Not supported.** Mask modifiers (`[m]`, `[m, src]`) cannot be applied to `split_by`.
 *
 * ### Return value
 * A `std::tuple` of SIMD vectors (of type `split_by_return_type<Chunks, V>`).
 * The sum of their lane counts equals the lane count of `V`.
*/
template <sizetype Chunks> 
constexpr inline auto split_by = options::functor<configurable_split_by_t<Chunks>::template impl>;

__RAZE_VX_NAMESPACE_END