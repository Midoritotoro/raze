#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/mask/Options.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/FindNextSet.h>
#  include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#endif // defined(raze_processor_x86)

#include <src/raze/algorithm/TailMask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_find_next_set_t : raze::options::conditional_callable<configurable_find_next_set_t, Options, not_null_option> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline i32 operator()(const M& x, i32 from) const noexcept {
        return options::dispatch_call(*this, x, from);
    }

    raze_nodiscard raze_always_inline auto operator()(algorithm::tail_mask_type auto const& x, i32 from) const noexcept {
        return (*this)(x(), from);
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, const M& x, i32 from) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        i32 index = 0;
        constexpr auto is_unsafe = Options::contains(not_null) && M::__chunks_count() == 1;

        i32 shift = from < 0 ? 0u : u32(from + 1);

        auto chunk_op = [&] <class Chunk> (const Chunk& chunk) raze_always_inline_lambda {
            if (shift >= i32(Chunk::size)) {
                index += Chunk::size;
                shift -= Chunk::size;
                return true;
            }

            auto r = find_next_set_<Abi::isa, Chunk::size, Value, is_unsafe>(ustorage(chunk), shift);
            index += r;

            if (r != Chunk::size) return false;

            shift = 0;
            return true;
        };

        x.__for_each_chunk_all_of(chunk_op);
        return index;
    }
};

constexpr inline auto find_next_set = options::functor<configurable_find_next_set_t>;

__RAZE_VX_NAMESPACE_END
