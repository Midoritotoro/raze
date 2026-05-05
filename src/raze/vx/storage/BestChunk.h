#pragma once 

#include <src/raze/vx/storage/Size.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_, class _Abi_, u64 _Elements_, intrin_type _Intrin_>
struct _Vector_wrapper {
    using vector_type = _Intrin_;
    using abi_type = _Abi_;
    using value_type = _Type_;

    static constexpr auto size = _Elements_;

    _Vector_wrapper(_Intrin_ __vector) noexcept : _data(__vector) {}

    _Vector_wrapper() noexcept = default;
    _Vector_wrapper(const _Vector_wrapper&) noexcept = default;
    _Vector_wrapper(_Vector_wrapper&&) noexcept = default;

    ~_Vector_wrapper() = default;

    _Vector_wrapper& operator=(const _Vector_wrapper&) noexcept = default;
    _Vector_wrapper& operator=(_Vector_wrapper&&) noexcept = default;

    raze_nodiscard raze_always_inline vector_type data() const noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline vector_type& data() noexcept {
        return _data;
    }
private:
    vector_type _data;
};

template <class _Type_, class _Abi_, i32 _Remaining_>
struct best_chunk {
    static constexpr auto __total_bytes = _Remaining_ * sizeof(_Type_);

    static constexpr auto __max_isa_width = __has_avx512f_support_v<_Abi_::isa> ? 512 :
        __has_avx_support_v<_Abi_::isa> ? 256 : __has_sse2_support_v<_Abi_::isa> ? 128 : 0;

    static constexpr auto __data_width = (__total_bytes >= 64) ? 512 :
        (__total_bytes >= 32) ? 256 : (__total_bytes >= 16) ? 128 : 0;

    static constexpr auto __width = (__data_width < __max_isa_width) ? __data_width : __max_isa_width;

    using _DeducedIntrin = type_traits::__deduce_simd_vector_type<_Type_, __width>;

    using type = std::conditional_t<__width != 0 && intrin_type<_DeducedIntrin>,
        _Vector_wrapper<_Type_, _Abi_, _Remaining_, _DeducedIntrin>, _Type_>; 
};

template <class _Type_, class _Abi_, i32 _Remaining_>
struct __build_tuple {
    using chunk_type = typename best_chunk<_Type_, _Abi_, _Remaining_>::type;

    static constexpr auto __chunk_elems = sizeof(chunk_type) / sizeof(_Type_);
    static constexpr auto __next = (_Remaining_ > __chunk_elems) ? (_Remaining_ - __chunk_elems) : 0;
    
    using type = _Simd_tuple_node<chunk_type, typename __build_tuple<_Type_, _Abi_, __next>::type>;
};

template <class _Type_, class _Abi_> 
struct __build_tuple<_Type_, _Abi_, 0> {
    using type = _Simd_tuple_nil; 
};

template <class _Type_, class _Abi_>
using _Simd_vector_tuple_type = typename __build_tuple<_Type_, _Abi_, _Abi_::size>::type;

__RAZE_VX_NAMESPACE_END
