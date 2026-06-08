#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _NumShifts_, sizetype _VectorBytes_, sizetype _ElementBytes_, typename _IdxType_>
consteval auto __make_rotate_left_table() noexcept {
    constexpr sizetype __num_indices = sizeof(_IdxType_) == 1 ? _VectorBytes_ : (_VectorBytes_ / sizeof(_IdxType_));
    constexpr sizetype __num_elements = _VectorBytes_ / _ElementBytes_;

    std::array<std::array<_IdxType_, __num_indices>, _NumShifts_> __table{};

    for (sizetype __shift = 0; __shift < _NumShifts_; ++__shift) {
        sizetype __actual_shift = __shift % __num_elements;

        for (sizetype __idx = 0; __idx < __num_indices; ++__idx) {
            if constexpr (sizeof(_IdxType_) == 1) {
                sizetype __dst_elem = __idx / _ElementBytes_;
                sizetype __src_elem = (__dst_elem + __actual_shift) % __num_elements;
                sizetype __src_byte = __src_elem * _ElementBytes_ + (__idx % _ElementBytes_);

                __table[__shift][__idx] = static_cast<_IdxType_>(__src_byte);
            }
            else {
                sizetype __src_elem = (__idx + __actual_shift) % __num_elements;

                __table[__shift][__idx] = static_cast<_IdxType_>(__src_elem);
            }
        }
    }
    return __table;
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_>
raze_nodiscard raze_always_inline _Intrin_ __make_rotate_left_idx(_Intrin_, i32 __sh) noexcept {
    constexpr auto __vector_bytes = sizeof(_Intrin_);
    constexpr auto __element_bytes = sizeof(_Type_);
    constexpr auto __num_shifts = __vector_bytes / __element_bytes;

    sizetype __shift = static_cast<sizetype>(__sh);

    if constexpr (__vector_bytes == 16) {
        alignas(16) static constexpr auto __table = __make_rotate_left_table<__num_shifts, __vector_bytes, __element_bytes, u8>();
        return _Load<_ISA_, _Intrin_>()(__table[__shift].data(), __aligned_policy{});
    }
    else if constexpr (__vector_bytes == 32) {
        if constexpr (__element_bytes >= 4) {
            using _IdxType_ = std::conditional_t<(__element_bytes == 8), u64, u32>;
            alignas(32) static constexpr auto __table = __make_rotate_left_table<__num_shifts, __vector_bytes, __element_bytes, _IdxType_>();
            return _Load<_ISA_, _Intrin_>()(__table[__shift].data(), __aligned_policy{});
        }
        else {
            alignas(32) static constexpr auto __table = __make_rotate_left_table<__num_shifts, __vector_bytes, __element_bytes, u8>();
            return _Load<_ISA_, _Intrin_>()(__table[__shift].data(), __aligned_policy{});
        }
    }
    else if constexpr (__vector_bytes == 64) {
        if constexpr (__element_bytes == 1) {
            alignas(64) static constexpr auto __table = __make_rotate_left_table<__num_shifts, __vector_bytes, __element_bytes, u8>();
            return _Load<_ISA_, _Intrin_>()(__table[__shift].data(), __aligned_policy{});
        }
        else if constexpr (__element_bytes == 2) {
            alignas(64) static constexpr auto __table = __make_rotate_left_table<__num_shifts, __vector_bytes, __element_bytes, u16>();
            return _Load<_ISA_, _Intrin_>()(__table[__shift].data(), __aligned_policy{});
        }
        else {
            using _IdxType_ = std::conditional_t<(__element_bytes == 8), u64, u32>;
            alignas(64) static constexpr auto __table = __make_rotate_left_table<__num_shifts, __vector_bytes, __element_bytes, _IdxType_>();
            return _Load<_ISA_, _Intrin_>()(__table[__shift].data(), __aligned_policy{});
        }
    }
    else {
        alignas(__vector_bytes) static constexpr auto __table = __make_rotate_left_table<__num_shifts, __vector_bytes, __element_bytes, u8>();
        return _Load<_ISA_, _Intrin_>()(__table[__shift].data(), __aligned_policy{});
    }
}


template <class _Pattern_>
raze_nodiscard raze_always_inline pattern_vector_t<_Pattern_> __rotate_left(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Simd_ = pattern_vector_t<_Pattern_>;
	using _Ret = decltype(__generic_shuffle(__x, __p));

	if constexpr (__is_fallback<_Ret>) {
		alignas(64) typename _Simd_::value_type __arr[_Simd_::size() * 2];

		vx::__store[vx::aligned](__arr, __x);
		vx::__store[vx::aligned](__arr + _Simd_::size(), __x);

		return vx::__load<_Simd_>[vx::aligned](__arr + __get_rotate_left_shift(__p));
	}
	else return __generic_shuffle(__x, __p);
}

template <simd_type _Simd_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __rotate_left_fallback(const _Simd_& __x, i32 __sh) noexcept {
	alignas(64) typename _Simd_::value_type __arr[_Simd_::size() * 2];

	vx::__store[vx::aligned](__arr, __x);
	vx::__store[vx::aligned](__arr + _Simd_::size(), __x);

	return vx::__load<_Simd_>[vx::aligned](__arr + __sh);
}

template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ __rotate_left(const _Simd_& __x, i32 __sh) noexcept {
	using _Abi_ = abi_t<_Simd_>;
	using _Value_ = typename _Simd_::value_type;

	if constexpr (native<_Simd_>) {
		using _Ret = decltype(__generic_shuffle_native<_Abi_::isa, _Value_>(
			__storage_unwrap(__x.template __get<0>()), __make_rotate_left_idx<_Abi_::isa,
			_Value_>(__storage_unwrap(__x.template __get<0>()), __sh)));
		
		if constexpr (__is_fallback<_Ret>) return __rotate_left_fallback(__x, __sh);
		else {
			auto __r = __x;
			auto& __storage = __r.template __get<0>();

			__storage = __generic_shuffle_native<_Abi_::isa, _Value_>(
				__storage_unwrap(__storage), __make_rotate_left_idx<_Abi_::isa,
				_Value_>(__storage_unwrap(__r.template __get<0>()), __sh));
			
			return __r;
		}
	}
	else {
		return __rotate_left_fallback(__x, __sh);
	}
}

__RAZE_VX_NAMESPACE_END
