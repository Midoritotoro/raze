#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shape/Concat.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_>
using abi_t = typename _Simd_::abi_type;

template <simd_type _Simd_>
constexpr auto __has_scalar_chunks_v = _Simd_::__has_scalar_chunks;

template <class ... _Types_>
concept same_abi_isa = ((abi_t<_Types_>::isa == abi_t<std::tuple_element_t<0, std::tuple<_Types_...>>>::isa) && ...);

template <class _Type_, class ... _Types_>
using concatenated_simd = simd<typename _Type_::value_type, resize_abi_t<typename _Type_::abi_type,
    abi_t<_Type_>::size + (abi_t<_Types_>::size + ...)>>;

template <class ... _Types_>
concept has_any_scalar_chunks = (__has_scalar_chunks_v<_Types_> || ...);

//template <class _Simd_>
//concept can_upcast_to_native_register = simd_type<_Simd_> && ();

template <class ... _Types_> 
concept effective_concatenation_available = same_abi_isa<_Types_...> && !has_any_scalar_chunks<_Types_...>;


// concat(tuple(__m512i, __m128i), tuple(__m512i, __m128i)) = tuple(__m512i, __m512i, __m256i) - cast __m128i to zmm, insert lower 3/4 of second tuple zmm, concat higher 128-bit second zmm with xmm
// concat(tuple(__m512i, __m128i), tuple(__m128i, __m256i)) = tuple(__m512i, __m512i) - just insert two regs(xmm, ymm) into casted __m128i to __m512i

template <class _Options_>
struct _Configurable_concat: raze::options::strict_elementwise_callable<_Configurable_concat, _Options_> {
    template <simd_type _Type_, class ... _Types_>
    raze_nodiscard raze_always_inline concatenated_simd<_Type_, _Types_...> operator()(
        const _Type_& __x, const _Types_& ... __xs) const noexcept
            requires(sizeof...(_Types_) > 0 && same_abi_isa<_Type_, _Types_...>)
    {
        return raze::options::__dispatch_call(*this, __x, __xs...);
    }

    template <class ... _Types_>
    static raze_always_inline auto deferred_call(auto __options, const _Types_& ... __xs) noexcept {
        using _Concatenated_ = concatenated_simd<_Types_...>;
        using _Value_ = typename _Concatenated_::value_type;
        using _Abi_ = typename _Concatenated_::abi_type;

        _Concatenated_ __result;

        if constexpr (effective_concatenation_available<_Types_...>) {
            __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
                __chunk = [&] () raze_always_inline_lambda {
                    // ...
                };
            });
        }
        else {
            alignas(64) _Value_ __arr[_Abi_::size];
            _Value_* __ptr = __arr;

            ([&]() raze_always_inline_lambda { __store[raze::vx::aligned](__ptr, __xs); __ptr += __xs.size(); }(), ...);
            __result = __load<_Concatenated_>[raze::vx::aligned](__arr);
        }

        return __result;
    }

    using callable_tag_type = _Configurable_concat;
};

__RAZE_VX_NAMESPACE_END
