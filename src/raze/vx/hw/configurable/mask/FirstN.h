#pragma once 

#include <raze/options/Options.h>
#include <src/raze/vx/Concepts.h>

#if defined(raze_processor_x86)
#  include <src/raze/vx/hw/x86/mask/operations/FirstN.h>
#endif // defined(raze_processor_x86)

#include <src/raze/utility/UninitializedTag.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Options_>
struct _Configurable_first_n : raze::options::conditional_callable<_Configurable_first_n, _Options_> {
    template <simd_mask_type _Type_>
    raze_nodiscard raze_always_inline _Type_ operator()(i32 __n, const options::as<_Type_>& __type) const noexcept {
        return raze::options::__dispatch_call(*this, __n, __type);
    }

    template <simd_mask_type _Type_>
    static raze_always_inline auto deferred_call(auto __options, i32 __n, const options::as<_Type_>&) noexcept {
        using _Mask_ = raze::options::fetch_t<raze::options::condition_key, _Options_>;
        static_assert(options::concepts::same_as<_Mask_, options::unknown_key>, "Not supported. ");

        using _Value_ = typename _Type_::value_type;
        using _Abi_ = typename _Type_::abi_type;

        _Type_ __x(uninitialized);

        if constexpr (_Type_::__chunks_count() == 1) {
            __x.__for_each_chunk([__n] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
                using _StorageType = std::remove_cvref_t<decltype(__storage_unwrap(__chunk))>;
                __chunk = _First_n<_Abi_::isa, _Chunk::size, _StorageType, _Value_>()(__n);
            });
        }
        else {
            i32 __remaining = __n;

            auto __chunk_op = [&__remaining] <class _Chunk> (_Chunk & __chunk) raze_always_inline_lambda {
                using _StorageType = std::remove_cvref_t<decltype(__storage_unwrap(__chunk))>;
                constexpr i32 __chunk_size = static_cast<i32>(_Chunk::size);

                if (__remaining > 0) {
                    i32 __chunk_n = (__remaining < __chunk_size) ? __remaining : __chunk_size;
                    __chunk = _First_n<_Abi_::isa, _Chunk::size, _StorageType, _Value_>()(__chunk_n);
                    __remaining -= __chunk_size;
                }
                else {
                    __chunk = _First_n<_Abi_::isa, _Chunk::size, _StorageType, _Value_>()(0);
                }
            };

            __x.__for_each_chunk(__chunk_op);
        }

        return __x;
    }

    using callable_tag_type = _Configurable_first_n;
};

constexpr inline auto __first_n = raze::options::functor<_Configurable_first_n>;

__RAZE_VX_NAMESPACE_END