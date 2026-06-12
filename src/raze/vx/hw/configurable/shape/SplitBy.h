#pragma once

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _Chunks_, simd_type _Simd_, sizetype _Index_>
using __split_by_simd_t = simd<typename _Simd_::value_type,
    resize_abi_t<abi_t<_Simd_>, ((_Index_ + 1) * _Chunks_ <= abi_t<_Simd_>::size)
    ? _Chunks_ : (abi_t<_Simd_>::size - _Index_ * _Chunks_)>>;

template <sizetype _Chunks_, simd_type _Simd_, sizetype... _Indices_>
using split_by_type = std::tuple<__split_by_simd_t<_Chunks_, _Simd_, _Indices_>...>;

template <sizetype _ChunkSize_, simd_type _Simd_, sizetype... _Indices_>
using split_by_type = std::tuple<__split_by_simd_t<_ChunkSize_, _Simd_, _Indices_>...>;

template <sizetype _ChunkSize_, simd_type _Simd_, class _Sequence_>
struct __split_by_return_impl;

template <sizetype _ChunkSize_, simd_type _Simd_, sizetype... _Indices_>
struct __split_by_return_impl<_ChunkSize_, _Simd_, std::integer_sequence<sizetype, _Indices_...>> {
    using type = split_by_type<_ChunkSize_, _Simd_, _Indices_...>;
};

template <sizetype _ChunkSize_, simd_type _Simd_>
using split_by_return_type = typename __split_by_return_impl<_ChunkSize_, _Simd_,
    std::make_integer_sequence<sizetype, (_Simd_::size() + _ChunkSize_ - 1) / _ChunkSize_>>::type;

template <sizetype _Chunks_>
struct _Configurable_split_by {
    template <class _Options_>
    struct __impl : raze::options::strict_elementwise_callable<__impl, _Options_> {
        template <simd_type _Simd_> requires(_Chunks_ > 0)
        raze_nodiscard raze_always_inline split_by_return_type<_Chunks_, _Simd_> operator()(const _Simd_& __x) const noexcept {
            return raze::options::__dispatch_call(*this, __x);
        }

        template <sizetype _ChunksCount_, simd_type _Simd_>
        static raze_always_inline auto __impl_split(const _Simd_& __x) noexcept {
            alignas(64) typename _Simd_::value_type __buffer[_Simd_::size()];
            vx::__store[vx::aligned](__buffer, __x);

            constexpr sizetype __count = (_Simd_::size() + _ChunksCount_ - 1) / _ChunksCount_;

            return [&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda {
                return std::tuple{ vx::__load<__split_by_simd_t<_ChunksCount_, _Simd_, _Indices_>>[vx::aligned](__buffer + _Indices_ * _ChunksCount_)... };
            }(std::make_integer_sequence<sizetype, __count>{});
        }

        template <simd_type _Simd_>
        static raze_always_inline auto deferred_call(auto, const _Simd_& __x) noexcept  {
            return __impl_split<_Chunks_>(__x);
        }

        using callable_tag_type = __impl;
    };
};

template <sizetype _Chunks_> constexpr inline auto __split_by = raze::options::functor<typename _Configurable_split_by<_Chunks_>::__impl>;

__RAZE_VX_NAMESPACE_END