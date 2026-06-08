#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _N_, sizetype _Elements_>
consteval std::array<std::array<u8, _Elements_>, _N_> __make_rotate_table() noexcept {
	std::array<std::array<u8, _Elements_>, _N_> __table;
	
	for (auto __i = 0; __i < _N_; ++__i)
		for (auto __j = 0; __j < _Elements_; ++__j)
			__table[__i][__j] = __j;
		
	return __table;
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_>
raze_nodiscard raze_always_inline _Intrin_ __make_rotate_left_idx(_Intrin_, i32 __sh) noexcept {
	constexpr auto __size = sizeof(_Intrin_) / sizeof(_Type_);

	if constexpr (sizeof(_Intrin_) == 16) {
		alignas(16) static constexpr auto __table = __make_rotate_table<__size, 16>();
		return _Load<_ISA_, _Intrin_>()(__table[__sh].data(), __aligned_policy{});
	}
	else if constexpr (sizeof(_Intrin_) == 32) {

	}
	else if constexpr (sizeof(_Intrin_) == 64) {

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
raze_nodiscard raze_always_inline _Simd_ __rotate_left_fallback(const _Simd_& __x, i32 __sh) noexcept {
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
		
		if constexpr (__is_fallback<_Simd_, _Ret>) return __rotate_left_fallback(__x, __sh);
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
