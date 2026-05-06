#pragma once 

#include <raze/vx/Abi.h>
#include <src/raze/vx/storage/Size.h>
#include <src/raze/vx/storage/ElementWise.h>
#include <src/raze/vx/storage/BestMaskChunk.h>
#include <src/raze/math/BitTest.h>
#include <src/raze/math/BitTestAndSet.h>
#include <src/raze/math/BitTestAndReset.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_, class _Abi_>
class _Mask_storage {
    static constexpr std::size_t __bytes = _Abi_::size * sizeof(_Type_);
    static constexpr bool __use_native = (__bytes == 16 && __has_sse2_support_v<_Abi_::isa>) ||
        (__bytes == 32 && __has_avx_support_v<_Abi_::isa>) || (__bytes == 64 && __has_avx512f_support_v<_Abi_::isa>);
public:
    using abi_type = _Abi_;
    using tuple_type = std::conditional_t<__use_native, typename best_mask_chunk<
        _Type_, _Abi_, _Abi_::size>::type, _Simd_mask_tuple_type<_Type_, _Abi_>>;

    _Mask_storage() noexcept = default;
    _Mask_storage(const _Mask_storage&) noexcept = default;
    _Mask_storage(_Mask_storage&&) noexcept = default;

    ~_Mask_storage() = default;

    _Mask_storage& operator=(const _Mask_storage&) noexcept = default;
    _Mask_storage& operator=(_Mask_storage&&) noexcept = default;

    raze_always_inline tuple_type storage() const noexcept {
        return _data;
    }

    raze_always_inline tuple_type& storage() noexcept {
        return _data;
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (__use_native)
            __f(_data, std::forward<_Args_>(__args)...);
        else
            __for_each_tuple(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (__use_native)
            return __f(_data, std::forward<_Args_>(__args)...);
        else
            return __for_each_tuple_all_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }


    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) noexcept {
        if constexpr (__use_native)
            return __f(_data, std::forward<_Args_>(__args)...);
        else
            return __for_each_tuple_any_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (__use_native)
            __f(_data, std::forward<_Args_>(__args)...);
        else
            __for_each_tuple(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_all_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (__use_native)
            return __f(_data, std::forward<_Args_>(__args)...);
        else
            return __for_each_tuple_all_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline auto __for_each_chunk_any_of(_Function_&& __f, _Args_&& ... __args) const noexcept {
        if constexpr (__use_native)
            return __f(_data, std::forward<_Args_>(__args)...);
        else
            return __for_each_tuple_any_of(_data, std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    raze_always_inline void __insert(i32 __i, bool __v) noexcept {
        if constexpr (__use_native) {
            if constexpr (intrin_type<tuple_type>) _Insert<_Abi_::isa>()(_data, __i, -__v);
            else if constexpr (std::is_same_v<std::remove_cvref_t<tuple_type>, bool>) _data = __v;
            else __v ? math::__bit_test_and_set(_data, __i) : math::__bit_test_and_reset(_data, __i);
        }
        else {
            __visit_chunk_by_index(_data, __i, [&] <class _Chunk> (_Chunk & __chunk, i32 __lane) raze_always_inline_lambda {
                if constexpr (intrin_type<_Chunk>) _Insert<_Abi_::isa>()(__chunk, __lane, -__v);
                else if constexpr (std::is_same_v<std::remove_cvref_t<_Chunk>, bool>) __chunk = __v;
                else __v ? math::__bit_test_and_set(__chunk, __lane) : math::__bit_test_and_reset(__chunk, __lane);
            });
        }
    }

    raze_always_inline bool __extract(i32 __i) const noexcept {
        if constexpr (__use_native) {
            if constexpr (intrin_type<typename tuple_type::unwrapped_type>) return bool(-_Extract<_Abi_::isa, _Type_>()(__storage_unwrap(_data), __i));
            else if constexpr (std::is_same_v<std::remove_cvref_t<tuple_type>, bool>) return __storage_unwrap(_data);
            else return math::__bit_test(__storage_unwrap(_data), __i);
        }
        else {
            bool __result{};

            __visit_chunk_by_index(_data, __i, [&] <class _Chunk> (const _Chunk & __chunk, i32 __lane) raze_always_inline_lambda {
                if constexpr (intrin_type<typename _Chunk::unwrapped_type>) __result = bool(-_Extract<_Abi_::isa, _Type_>()(__storage_unwrap(__chunk), __lane));
                else if constexpr (std::is_same_v<std::remove_cvref_t<_Chunk>, bool>) __result = __storage_unwrap(__chunk);
                else __result = math::__bit_test(__storage_unwrap(__chunk), __lane);
            });

            return __result;
        }
    }
private:
    tuple_type _data;
};

__RAZE_VX_NAMESPACE_END
