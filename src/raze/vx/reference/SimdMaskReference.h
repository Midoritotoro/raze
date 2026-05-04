#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/utility/Assert.h>
#include <src/raze/vx/Concepts.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Mask_>
class _Simd_mask_reference {
	using __element_type = typename _Mask_::value_type;
public:
	using value_type = bool;

	_Simd_mask_reference(_Mask_& __mask, i32 __i) noexcept :
		_reference(__mask),
		_index(__i)
	{
		raze_assert(__i >= 0 && __i < _Mask_::size());
	}

	raze_always_inline operator value_type() const noexcept {
		return __read();
	}

	raze_always_inline _Simd_mask_reference operator=(value_type __value) && noexcept {
		__write(__value);
		return { _reference, _index };
	}

	raze_always_inline value_type operator++(int) && noexcept {
		auto __value = __read();
		const auto __old_value = __value;
		__write(++__value);
		return __old_value;
	}

	raze_always_inline _Simd_mask_reference operator++() && noexcept {
		__write(++__read());
		return { _reference, _index };
	}

	raze_always_inline value_type operator--(int) && noexcept {
		auto __value = __read();
		const auto __old_value = __value;
		__write(--__value);
		return __old_value;
	}

	raze_always_inline _Simd_mask_reference operator--() && noexcept {
		__write(--__read());
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator&=(value_type __other) && noexcept {
		__write(__read() & __other);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator|=(value_type __other) && noexcept {
		__write(__read() | __other);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator^=(value_type __other) && noexcept {
		__write(__read() ^ __other);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator>>=(i32 __shift) && noexcept {
		__write(__read() >> __shift);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator<<=(i32 __shift) && noexcept {
		__write(__read() << __shift);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator-=(value_type __other) && noexcept {
		__write(__read() - __other);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator+=(value_type __other) && noexcept {
		__write(__read() + __other);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator*=(value_type __other) && noexcept {
		__write(__read() * __other);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator/=(value_type __other) && noexcept {
		__write(__read() / __other);
		return { _reference, _index };
	}

	raze_always_inline _Simd_mask_reference operator%=(value_type __other) && noexcept {
		__write(__read() % __other);
		return { _reference, _index };
	}

	raze_always_inline friend void swap(_Simd_mask_reference& __left, _Simd_mask_reference& __right) noexcept {
		value_type __temp = static_cast<_Simd_mask_reference&&>(__left);
		static_cast<_Simd_mask_reference&&>(__left) = static_cast<value_type>(__right);
		static_cast<_Simd_mask_reference&&>(__right) = std::move(__temp);
	}

	raze_always_inline friend void swap(_Simd_mask_reference& __left, value_type& __right) noexcept {
		value_type __temp = static_cast<_Simd_mask_reference&&>(__left);
		static_cast<_Simd_mask_reference&&>(__left) = std::move(__right);
		__right = std::move(__temp);
	}

	raze_always_inline friend void swap(value_type& __left, _Simd_mask_reference& __right) noexcept {
		value_type __temp = static_cast<_Simd_mask_reference&&>(__right);
		static_cast<_Simd_mask_reference&&>(__right) = std::move(__left);
		__left = std::move(__temp);
	}
private:
	raze_always_inline value_type __read() const noexcept {
		return _reference.__extract(_index);
	}

	raze_always_inline void __write(value_type __value) noexcept {
		_reference.__insert(_index, __value);
	}

	_Mask_& _reference;
	i32 _index = 0;
};

__RAZE_VX_NAMESPACE_END
