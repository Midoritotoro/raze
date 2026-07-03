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
        raze_debug_assert(__n < _Type_::size() && __k <= _Type_::size() && __n <= __k);
        return raze::options::__dispatch_call(*this, __x, __n, __k);
    }

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
            auto __ch1 = __x.template __get<0>();
            auto __ch2 = __x.template __get<1>();

            using _Ch1 = decltype(__ch1);
            using _Ch2 = decltype(__ch2);

            if (__k <= _Ch1::size) return _Is_contiguous<_Abi_::isa, _Ch1::size, _Value_>()(__storage_unwrap(__ch1), __n, __k);
            return _Is_contiguous<_Abi_::isa, _Ch1::size, _Value_>()(__storage_unwrap(__ch1), __n, __k) &&
                _Is_contiguous<_Abi_::isa, _Ch2::size, _Value_>()(__storage_unwrap(__ch2), 0, __k - _Ch1::size);
        }
        else {
            return [&] <sizetype ... __I> (std::integer_sequence<size_t, __I...>) raze_always_inline_lambda {
                return ([&] (auto __i) raze_always_inline_lambda {
                    auto __ch = __x.template __get<__i>();
                    constexpr auto __size = decltype(__ch)::size;

                    if (__n >= __size) {
                        __n -= __size;
                        __k -= __size;
                        return true;
                    }

                    if (__k <= 0) return true;
                    
                    const auto __end = __k < __size ? __k : __size;
                    const auto __r = _Is_contiguous<_Abi_::isa, __size, _Value_>()(__storage_unwrap(__ch), __n, __end);

                    __n = 0;
                    __k -= __size;

                    return __r;
                }(std::integral_constant<sizetype, __I>{}) && ...);
            }(std::make_integer_sequence<sizetype, _Type_::__chunks_count()>{});
        }
    }

    using callable_tag_type = _Configurable_is_contiguous;
};

__RAZE_VX_NAMESPACE_END
