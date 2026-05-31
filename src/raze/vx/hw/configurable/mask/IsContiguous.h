#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/IsContiguous.h>
#endif // defined(raze_processor_x86)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_is_contiguous: raze::options::strict_elementwise_callable<_Configurable_is_contiguous, _Options_> {
    template <simd_mask_type _Type_>
    raze_nodiscard raze_always_inline bool operator()(const _Type_& __x, i32 __n, i32 __k) const noexcept {
        raze_debug_assert(__n < _Type_::size() && __k < _Type_::size() && __n <= __k);
        return raze::options::__dispatch_call(*this, __x, __n, __k);
    }

    // 16 8
    // 17 20
    //  
    template <simd_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, const _Type_& __x, i32 __n, i32 __k) noexcept {
        static_assert(raze::options::concepts::same_as<raze::options::fetch_t<raze::options::condition_key, _Options_>,
            raze::options::unknown_key>, "is_contiguous does not support conditional masks via []. ");

        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        if constexpr (_Type_::__is_native() || _Type_::size() == 1) {
            return __x.__for_each_chunk_any_of([&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
                return _Is_contiguous<_Abi_::isa, _Chunk::size, _Value_>()(__storage_unwrap(__chunk), __n, __k);
            });
        }
        else if constexpr (_Type_::__chunks_count() == 2) {
            auto __chunk1 = __x.template __get<0>();
            auto __chunk2 = __x.template __get<1>();

            using _Ch1 = decltype(__chunk1);
            using _Ch2 = decltype(__chunk2);

            return _Is_contiguous<_Abi_::isa, _Ch1::size, _Value_>()(__storage_unwrap(__chunk1), __n, __k)
                && (__k > _Ch1::size) && _Is_contiguous<_Abi_::isa, _Ch2::size, _Value_>()(__storage_unwrap(__chunk2), __n - _Ch1::size, __k - _Ch1::size);
        }
        else {
            return __x.__for_each_chunk_all_of([&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
                if (__n >= _Chunk::size) {
                    __n -= _Chunk::size;
                    __k -= _Chunk::size;
                    return true;
                }

                if (__k <= 0) return true;

                const auto __begin = __n;
                const auto __end = __k < _Chunk::size ? __k : _Chunk::size;

                __n = 0;
                __k -= _Chunk::size;

                return _Is_contiguous<_Abi_::isa, _Chunk::size, _Value_>()(
                    __storage_unwrap(__chunk), __begin, __end);
            });
        }
    }

    using callable_tag_type = _Configurable_is_contiguous;
};

__RAZE_VX_NAMESPACE_END
