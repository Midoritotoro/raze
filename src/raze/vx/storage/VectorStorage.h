#pragma once 

#include <raze/vx/Abi.h>
#include <src/raze/vx/storage/Size.h>
#include <src/raze/vx/storage/ElementWise.h>
#include <src/raze/vx/storage/BestChunk.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_, class _Abi_>
class _Vector_storage {
    static constexpr std::size_t __bytes = _Abi_::size * sizeof(_Type_);
    static constexpr bool __use_native = (__bytes == 16 && __has_sse2_support_v<_Abi_::isa>) ||
        (__bytes == 32 && __has_avx_support_v<_Abi_::isa>) || (__bytes == 64 && __has_avx512f_support_v<_Abi_::isa>);

public:
    using abi_type = _Abi_;
    using tuple_type = std::conditional_t<__use_native, typename best_chunk<
        _Type_, _Abi_, _Abi_::size>::type, _Simd_vector_tuple_type<_Type_, _Abi_>>;
    
    _Vector_storage() noexcept = default;
    _Vector_storage(const _Vector_storage&) noexcept = default;
    _Vector_storage(_Vector_storage&&) noexcept = default;

    ~_Vector_storage() = default;

    _Vector_storage& operator=(const _Vector_storage&) noexcept = default;
    _Vector_storage& operator=(_Vector_storage&&) noexcept = default;

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

    raze_always_inline void __insert(i32 __i, _Type_ __value) noexcept {
        raze_debug_assert(__i >= 0 && __i < _Abi_::size);

        if constexpr (__use_native) _Insert<_Abi_::isa>()(__storage_unwrap(_data), __i, __value);
        else __visit_chunk_by_index(_data, __i, [&](auto& __chunk, i32 __lane) raze_always_inline_lambda {
            _Insert<_Abi_::isa>()(__storage_unwrap(__chunk), __lane, __value);
        });
    }

    raze_nodiscard raze_always_inline _Type_ __extract(i32 __i) const noexcept {
        raze_debug_assert(__i >= 0 && __i < _Abi_::size);
        _Type_ __result{};

        if constexpr (__use_native) __result = _Extract<_Abi_::isa, _Type_>()(__storage_unwrap(_data), __i);
        else __visit_chunk_by_index(_data, __i, [&](const auto& __chunk, i32 __lane) raze_always_inline_lambda {
            __result = _Extract<_Abi_::isa, _Type_>()(__storage_unwrap(__chunk), __lane);
        });

        return __result;

    }
private:
    tuple_type _data;
};

__RAZE_VX_NAMESPACE_END
