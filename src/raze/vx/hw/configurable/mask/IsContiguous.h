#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/IsContiguous.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_is_contiguous_t: options::conditional_callable<configurable_is_contiguous_t, Options> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline bool operator()(const M& x, i32 n, i32 k) const noexcept {
        raze_debug_assert(n < M::size() && k <= M::size() && n <= k);
        return options::dispatch_call(*this, x, n, k);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x, i32 n, i32 k) const noexcept {
        return (*this)(x(), n, k);
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, const M& x, i32 n, i32 k) noexcept {
        static_assert(!options::complete_mask<options::fetch_t<options::condition_key, Options>>,
             "is_contiguous does not support conditional masks via []. ");

        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        if constexpr (M::is_native() || M::size() == 1) {
            return x.__for_each_chunk_any_of([&] <class Chunk> (const Chunk& chunk) raze_always_inline_lambda {
                return is_contiguous_<Abi::isa, Chunk::size, Value>(ustorage(chunk), n, k);
            });
        }
        else if constexpr (M::__chunks_count() == 2) {
            auto ch1 = x.template __get<0>();
            auto ch2 = x.template __get<1>();

            using Ch1 = decltype(ch1);
            using Ch2 = decltype(ch2);

            if (k <= Ch1::size) return is_contiguous_<Abi::isa, Ch1::size, Value>(ustorage(ch1), n, k);
            return is_contiguous_<Abi::isa, Ch1::size, Value>(ustorage(ch1), n, k) &&
                is_contiguous_<Abi::isa, Ch2::size, Value>(ustorage(ch2), 0, k - Ch1::size);
        }
        else {
            return [&] <sizetype ... I> (std::integer_sequence<size_t, I...>) raze_always_inline_lambda {
                return ([&] (auto i) raze_always_inline_lambda {
                    auto ch = x.template __get<i>();
                    constexpr auto size = decltype(ch)::size;

                    if (n >= size) {
                        n -= size;
                        k -= size;
                        return true;
                    }

                    if (k <= 0) return true;
                    
                    const auto end = k < size ? k : size;
                    const auto r = is_contiguous_<Abi::isa, size, Value>(ustorage(ch), n, end);

                    n = 0;
                    k -= size;

                    return r;
                }(std::integral_constant<sizetype, I>{}) && ...);
            }(std::make_integer_sequence<sizetype, M::__chunks_count()>{});
        }
    }
};

constexpr inline auto is_contiguous = options::functor<configurable_is_contiguous_t>;

__RAZE_VX_NAMESPACE_END
