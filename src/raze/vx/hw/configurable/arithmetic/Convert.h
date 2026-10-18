#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/Storage.h>
#include <tuple>
#include <utility>
#include <type_traits>
#include <bit>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/arithmetic/Convert.h>
#endif // defined(raze_processor_x86)

__RAZE_VX_NAMESPACE_BEGIN

struct saturated_mode {};
inline constexpr auto saturated = raze::options::flag(saturated_mode{});
struct saturated_option : raze::options::exact_option<saturated> {};

template <class R>
struct reg_count {
    static constexpr sizetype value = 1;
};

template <class... Ts>
struct reg_count<std::tuple<Ts...>> {
    static constexpr sizetype value = sizeof...(Ts);
};

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
        using Abi = typename V::abi_type;

        return[] <sizetype... Is> (std::index_sequence<Is...>) {
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

    if constexpr (vx::is_std_tuple_v<OutRegs>) {
        return[] <sizetype... Ks> (std::index_sequence<Ks...>) {
            return ((sizeof(std::tuple_element_t<Ks, OutRegs>) ==
                sizeof(std::remove_cvref_t<decltype(ustorage(std::declval<Result&>().template __get<offset + Ks>()))>)) && ...);
        }(std::make_index_sequence<std::tuple_size_v<OutRegs>>{});
    }
    else {
        using DstChunk = std::remove_cvref_t<decltype(ustorage(std::declval<Result&>().template __get<offset>()))>;
        return sizeof(OutRegs) == sizeof(DstChunk);
    }
}


template <class V, class Result>
consteval bool check_all_chunk_sizes_match() {
    using To = typename Result::value_type;

    constexpr sizetype num_src = V::__chunks_count();
    constexpr sizetype num_dst = Result::__chunks_count();
    constexpr sizetype total_produced = get_total_dst_regs<V, To>();

    if constexpr (num_dst != total_produced) return false;
    else {
        return [] <sizetype... Is> (std::index_sequence<Is...>) {
            return (chunk_size_matches<Is, V, Result>() && ...);
        }(std::make_index_sequence<num_src>{});
    }
}

template <bool Saturated, sizetype I, class V, class T, class Dst>
raze_always_inline void convert_chunk_into(Dst& dst, const V& x) noexcept {
    using Value = typename V::value_type;
    using Abi = typename V::abi_type;

    constexpr sizetype offset = get_chunk_dst_offset<I, V, T>();
    auto in_reg = get_src_chunk<I>(x);
    auto r = convert_<Abi::isa, Saturated, Value, T>(in_reg);
    assign_regs<offset>(dst, std::move(r)); 
}

template <arithmetic_type T>
struct configurable_convert_t {
    template <class Options>
    struct convert : options::conditional_callable<convert, Options, saturated_option> {
        template <simd_type V>
        raze_nodiscard raze_always_inline auto operator()(const V& x) const noexcept {
            return options::dispatch_call(*this, x);
        }

        template <simd_type V>
        static raze_always_inline auto deferred_call(auto, const V& x) noexcept {
            using Abi = typename V::abi_type;
            using ResultSimd = simd<T, Abi>;

            ResultSimd res(uninitialized);

            constexpr sizetype num_src_chunks = V::__chunks_count();
            constexpr bool     can_fast_convert = check_all_chunk_sizes_match<V, ResultSimd>();

            if constexpr (can_fast_convert) {
                [&] <sizetype... Is> (std::index_sequence<Is...>) raze_always_inline_lambda {
                    (convert_chunk_into<Options::contains(saturated), Is, V, T>(res, x), ...);
                }(std::make_index_sequence<num_src_chunks>{});
            }
            else {
                for (sizetype i = 0; i < Abi::size; ++i)
                    res[i] = static_cast<T>(x[i]);
            }

            return res;
        }
    };
};

template <arithmetic_type T>
constexpr inline auto convert = options::functor<configurable_convert_t<T>::template convert>;

__RAZE_VX_NAMESPACE_END 
