#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shuffle/Shuffle.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Simd_>
concept index_simd_type = simd_type<_Simd_> && std::is_unsigned_v<typename _Simd_::value_type>;

struct across_chunks_mode {};
constexpr auto across_chunks = raze::options::flag(across_chunks_mode{});
struct across_chunks_option : raze::options::exact_option<across_chunks> {};

template <class _Options_>
struct _Configurable_shuffle: raze::options::strict_elementwise_callable<_Configurable_shuffle, _Options_, across_chunks_option> {
    //template <simd_type _Type_, index_simd_type _Index_>
    //raze_nodiscard raze_always_inline _Type_ operator()(
    //    const _Type_& __x, const _Index_& __index) const noexcept
    //        requires(_Type_::size() == _Index_::size())
    //{
    //    return raze::options::__dispatch_call(*this, __x, __index);
    //}

    template <simd_type _Type_, sizetype ... _Indices_>
    raze_nodiscard raze_always_inline _Type_ operator()(const _Type_& __x, 
        std::integer_sequence<sizetype, _Indices_...> __indices) const noexcept
            requires(_Type_::size() == sizeof...(_Indices_))
    {
        return raze::options::__dispatch_call(*this, __x, __indices);
    }

    //template <simd_type _Type_, index_simd_type _Index_>
    //static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, const _Index_& __index) noexcept {
    //    
    //}

    template <simd_type _Type_, sizetype ... _Indices_>
    raze_nodiscard static raze_always_inline auto deferred_call(auto __options,
        const _Type_& __x, std::integer_sequence<sizetype, _Indices_...> __indices) noexcept
    {
        return __shuffle(__x, _Shuffle_pattern<_Type_, _Indices_...>{});
    }

    using callable_tag_type = _Configurable_shuffle;
};

__RAZE_VX_NAMESPACE_END
