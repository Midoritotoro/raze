#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _VectorBytes_, sizetype _ElementBytes_, typename _IdxType_>
consteval auto __make_rotate_shuffle_table() noexcept {
    constexpr sizetype __num_elements = _VectorBytes_ / _ElementBytes_;
    constexpr sizetype __idx_bytes = sizeof(_IdxType_);
    constexpr sizetype __num_indices = _VectorBytes_ / __idx_bytes;

    constexpr sizetype __chunks_per_element = _ElementBytes_ / __idx_bytes;
    constexpr sizetype __num_shifts = __num_elements;

    std::array<std::array<_IdxType_, __num_indices>, __num_shifts> __table{};

    for (sizetype __shift = 0; __shift < __num_shifts; ++__shift) {
        sizetype __actual_shift = __shift % __num_elements;

        for (sizetype __idx = 0; __idx < __num_indices; ++__idx) {
            sizetype __dst_elem = __idx / __chunks_per_element;
            sizetype __chunk_offset = __idx % __chunks_per_element;
            sizetype __src_elem = (__dst_elem + __actual_shift) % __num_elements;
            sizetype __src_chunk = __src_elem * __chunks_per_element + __chunk_offset;

            __table[__shift][__idx] = static_cast<_IdxType_>(__src_chunk);
        }
    }
    return __table;
}

template <intrin_type _Intrin_, class _IdxType_>
struct _Rotate_indices {
    using index_type = _IdxType_;
    _Rotate_indices(_Intrin_ __x) noexcept : _data(__x) {}

    _Rotate_indices(const _Rotate_indices&) noexcept = default;
    _Rotate_indices(_Rotate_indices&&) noexcept = default;

    _Rotate_indices& operator=(const _Rotate_indices&) noexcept = default;
    _Rotate_indices& operator=(_Rotate_indices&&) noexcept = default;

    raze_always_inline operator _Intrin_() const noexcept {
        return _data;
    }

    raze_always_inline _Intrin_ data() const noexcept {
        return _data;
    }

    _Intrin_ _data;
};

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_>
raze_nodiscard raze_always_inline auto __make_rotate_left_idx(_Intrin_, i32 __sh) noexcept {
    constexpr auto __vector_bytes = sizeof(_Intrin_);
    constexpr auto __element_bytes = sizeof(_Type_);

    if constexpr (sizeof(_Intrin_) == 16) {
        alignas(16) static constexpr auto __table = __make_rotate_shuffle_table<__vector_bytes, __element_bytes, u8>();
        return _Rotate_indices<_Intrin_, u8> { _Load<_ISA_, _Intrin_>()(__table[__sh].data(), __aligned_policy{}) };
    }
    else if constexpr (__vector_bytes == 32) {
        using _IdxType = typename IntegerForSizeof<_Type_>::Unsigned;

        if constexpr (__has_avx2_support_v<_ISA_> && !(__has_avx512bw_support_v<_ISA_> && __has_avx512vl_support_v<_ISA_>)) {
            if constexpr (sizeof(_IdxType) >= 4) {
                alignas(32) static constexpr auto __table = __make_rotate_shuffle_table<__vector_bytes, __element_bytes, u32>();
                return _Rotate_indices<_Intrin_, u32>{ _Load<_ISA_, _Intrin_>()(__table[__sh].data(), __aligned_policy{}) };
            }
            else {
                alignas(32) static constexpr auto __table = __make_rotate_shuffle_table<__vector_bytes, __element_bytes, u8>();
                return _Rotate_indices<_Intrin_, u8>{ _Load<_ISA_, _Intrin_>()(__table[__sh].data(), __aligned_policy{}) };
            }
        }
        else {
            alignas(32) static constexpr auto __table = __make_rotate_shuffle_table<__vector_bytes, __element_bytes, _IdxType>();
            return _Rotate_indices<_Intrin_, _IdxType>{ _Load<_ISA_, _Intrin_>()(__table[__sh].data(), __aligned_policy{}) };
        }
    }
    else if constexpr (__vector_bytes == 64) {
        using _IdxType = typename IntegerForSizeof<_Type_>::Unsigned;

        alignas(64) static constexpr auto __table = __make_rotate_shuffle_table<__vector_bytes, __element_bytes, _IdxType>();
        return _Rotate_indices<_Intrin_, _IdxType>{ _Load<_ISA_, _Intrin_>()(__table[__sh].data(), __aligned_policy{}) };
    }
}

template <simd_type _Simd_, class _Int_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __rotate_left_fallback(const _Simd_& __x, _Int_ __sh) noexcept {
    alignas(64) typename _Simd_::value_type __arr[_Simd_::size() * 2];

    vx::__store[vx::aligned](__arr, __x);
    vx::__store[vx::aligned](__arr + _Simd_::size(), __x);

    return vx::__load<_Simd_>[vx::aligned](__arr + __sh);
}

template <class _Pattern_>
raze_nodiscard raze_always_inline pattern_vector_t<_Pattern_> __rotate_left(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Ret = decltype(__generic_shuffle(__x, __p));
	if constexpr (__is_fallback<_Ret>) return __rotate_left_fallback(__x, __get_rotate_left_shift(__p));
	else return __generic_shuffle(__x, __p);
}

template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ __rotate_left(const _Simd_& __x, i32 __sh) noexcept {
	using _Abi_ = abi_t<_Simd_>;
	using _Value_ = typename _Simd_::value_type;

	if constexpr (native<_Simd_>) {
		using _Ret = decltype(__generic_shuffle_native<_Abi_::isa, typename decltype(__make_rotate_left_idx<_Abi_::isa,
            _Value_>(__storage_unwrap(__x.template __get<0>()), __sh))::index_type>(
			__storage_unwrap(__x.template __get<0>()), __make_rotate_left_idx<_Abi_::isa,
			_Value_>(__storage_unwrap(__x.template __get<0>()), __sh).data()));
		
		if constexpr (__is_fallback<_Ret>) return __rotate_left_fallback(__x, __sh);
		else {
			auto __r = __x;
			auto& __storage = __r.template __get<0>();

            const auto __rotate_indices = __make_rotate_left_idx<_Abi_::isa,
                _Value_>(__storage_unwrap(__r.template __get<0>()), __sh);

            using _IndexType = typename decltype(__rotate_indices)::index_type;
			__storage = __generic_shuffle_native<_Abi_::isa, _IndexType>(__storage_unwrap(__storage), __rotate_indices.data());
			
			return __r;
		}
	}
	else {
		return __rotate_left_fallback(__x, __sh);
	}
}

__RAZE_VX_NAMESPACE_END
