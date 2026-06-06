#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/shape/Concat.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_, class ... _Types_>
using concatenated_simd = simd<typename _Type_::value_type, resize_abi_t<typename _Type_::abi_type,
    abi_t<_Type_>::size + (abi_t<_Types_>::size + ...)>>;

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
        
        /*using _Tup = std::tuple<_Types_...>;
        _Tup __tup = std::make_tuple(__xs...);

        using _BestWrappedChunk = typename best_chunk<_Value_, _Abi_, _Abi_::size>::type;
        using _BestChunk = typename _BestWrappedChunk::unwrapped_type;*/

        /*if constexpr (effective_concatenation_available<_Types_...>) {
            (([&] <u64 ... __I1> () raze_always_inline_lambda {
                auto __current_tuple = std::get<__I1>(__tup);
                using _SimdType = decltype(__current_tuple);

                constexpr auto __size1 = _SimdType::size();
                constexpr auto __chunks_count = __simd_tuple_size<typename typename _SimdType::storage_type::tuple_type>::value;

                _BestChunk __src;

                [&] <u64 ... __I2> (std::integer_sequence<sizetype, __I2...>) raze_always_inline_lambda {
                    ([&](auto __current) raze_always_inline_lambda {
                        auto& __chunk = __get<__current>(__result);
                        auto __temp_chunk = __get<__current % __chunks_count>(__current_tuple);
                        __chunk.template insert<__current>(__temp_chunk.data());
                    }(std::integral_constant<std::size_t, __I2>{}), ...);
                }(std::make_integer_sequence<sizetype, __chunks_count>{});
            }(std::make_index_sequence<std::tuple_size_v<_Tup>>{})));
        }
        else*/ {
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
