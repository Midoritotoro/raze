#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/FirstN.h>
#endif // defined(raze_processor_x86)

#include <src/raze/utility/UninitializedTag.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class Options>
struct configurable_first_n_t : options::conditional_callable<configurable_first_n_t, Options> {
    template <simd_mask_type M>
    raze_nodiscard raze_always_inline M operator()(i32 n, const options::as<M>& type) const noexcept {
        return options::dispatch_call(*this, n, type);
    }

    template <simd_mask_type M>
    static raze_always_inline auto deferred_call(auto opts, i32 n, const options::as<M>&) noexcept {
        using Mask = options::fetch_t<options::condition_key, Options>;
        static_assert(!options::complete_mask<Mask>, "Not supported. ");

        using Value = typename M::value_type;
        using Abi = typename M::abi_type;

        M x(uninitialized);

        if constexpr (M::__chunks_count() == 1) {
            x.__for_each_chunk([n] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
                using StorageType = std::remove_cvref_t<decltype(ustorage(chunk))>;
                chunk = first_n_<Abi::isa, Chunk::size, StorageType, Value>(n);
            });
        }
        else {
            i32 remaining = n;

            auto chunk_op = [&remaining] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
                using StorageType = std::remove_cvref_t<decltype(ustorage(chunk))>;
                constexpr i32 chunk_size = static_cast<i32>(Chunk::size);

                if (remaining > 0) {
                    i32 chunk_n = (remaining < chunk_size) ? remaining : chunk_size;
                    chunk = first_n_<Abi::isa, Chunk::size, StorageType, Value>(chunk_n);
                    remaining -= chunk_size;
                }
                else {
                    chunk = first_n_<Abi::isa, Chunk::size, StorageType, Value>(0);
                }
            };

            x.__for_each_chunk(chunk_op);
        }

        return x;
    }
};

constexpr inline auto first_n = options::functor<configurable_first_n_t>;

__RAZE_VX_NAMESPACE_END