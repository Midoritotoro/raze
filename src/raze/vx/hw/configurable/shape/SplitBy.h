#pragma once

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype Chunks, simd_type V, sizetype Index>
using split_by_simd_t = simd<
    typename V::value_type, resize_abi_t<abi_t<V>,
        ((Index + 1) * ((abi_t<V>::size + Chunks - 1) / Chunks)
            <= abi_t<V>::size
        )
        ?
        ((abi_t<V>::size + Chunks - 1) / Chunks)
        :
        (
            abi_t<V>::size -
            Index *
            ((abi_t<V>::size + Chunks - 1) / Chunks)
        )
    >
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
        template <simd_type V> requires(Chunks > 0)
        raze_nodiscard raze_always_inline split_by_return_type<Chunks, V> operator()(const V& x) const noexcept {
            return options::dispatch_call(*this, x);
        }

        template <sizetype ChunksCount, simd_type V>
        static raze_always_inline auto impl_split(const V& x) noexcept {
            constexpr sizetype count = ChunksCount;
            constexpr sizetype chunk_size = (V::size() + count - 1) / count;

            if constexpr (ChunksCount == V::__chunks_count()) {
                const auto get_simd = [&] <class Vec> (Vec, auto i) raze_always_inline_lambda {
                    Vec vec;
                    vec.template __get<i>() = ustorage(x.template __get<i>());
                    return vec;
                };

                return [&] <sizetype... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
                    return std::tuple{ get_simd(split_by_simd_t<ChunksCount, V, I>{}, std::integral_constant<sizetype, I>{})...
                };
                }(std::make_integer_sequence<sizetype, count>{});
            }
            else {
                alignas(64) typename V::value_type buffer[V::size()];
                vx::store[vx::aligned](buffer, x);

                return [&] <sizetype... I> (std::integer_sequence<sizetype, I...>) raze_always_inline_lambda {
                    return std::tuple{ vx::load<split_by_simd_t<ChunksCount, V, I>>[vx::aligned](buffer + I * chunk_size)... };
                }(std::make_integer_sequence<sizetype, count>{});
            }
        }

        template <simd_type V>
        static raze_always_inline auto deferred_call(auto, const V& x) noexcept  {
            return impl_split<Chunks>(x);
        }
    };
};

template <sizetype Chunks> constexpr inline auto split_by = options::functor<configurable_split_by_t<Chunks>::template impl>;

__RAZE_VX_NAMESPACE_END