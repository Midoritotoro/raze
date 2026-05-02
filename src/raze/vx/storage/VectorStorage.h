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
    using tuple_type = std::conditional_t<__use_native, type_traits::__deduce_simd_vector_type<_Type_, 
        (_Abi_::size * sizeof(_Type_) * 8)>, _Simd_vector_tuple_type<_Type_, _Abi_>>;

    _Vector_storage() noexcept = default;
    _Vector_storage(const _Vector_storage&) noexcept = default;
    _Vector_storage(_Vector_storage&&) noexcept = default;

    ~_Vector_storage() = default;

    _Vector_storage& operator=(const _Vector_storage&) noexcept = default;
    _Vector_storage& operator=(_Vector_storage&&) noexcept = default;

    raze_always_inline operator tuple_type() const noexcept {
        return _data;
    }

    raze_always_inline operator tuple_type&() noexcept {
        return _data;
    }

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
private:
    tuple_type _data;
};

__RAZE_VX_NAMESPACE_END
