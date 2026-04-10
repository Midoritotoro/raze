#pragma once 

#include <src/raze/vx/SimdIntegralTypesCheck.h>
#include <src/raze/math/BitTestAndSet.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Type_>
class _Bit_reference {
public:
	static_assert(std::is_integral_v<_Type_>);

	using value_type = bool;

	_Bit_reference(_Type_& __value, int32 __i) noexcept :
		_reference(__value),
		_index(__i)
	{}

	raze_always_inline operator value_type() const noexcept {
		return __read();
	}

	raze_always_inline _Bit_reference operator=(value_type __value) && noexcept {
		__write(__value);
		return { _reference, _index };
	}

	raze_always_inline value_type operator++(int) && noexcept {
		auto __value = __read();
		const auto __old_value = __value;
		__write(++__value);
		return __old_value;
	}

	raze_always_inline _Bit_reference operator++() && noexcept {
		__write(++__read());
		return { _reference, _index };
	}

	raze_always_inline value_type operator--(int) && noexcept {
		auto __value = __read();
		const auto __old_value = __value;
		__write(--__value);
		return __old_value;
	}

	raze_always_inline _Bit_reference operator--() && noexcept {
		__write(--__read());
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator&=(value_type __other) && noexcept {
		__write(__read() & __other);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator|=(value_type __other) && noexcept {
		__write(__read() | __other);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator^=(value_type __other) && noexcept {
		__write(__read() ^ __other);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator>>=(int32 __shift) && noexcept {
		__write(__read() >> __shift);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator<<=(int32 __shift) && noexcept {
		__write(__read() << __shift);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator-=(value_type __other) && noexcept {
		__write(__read() - __other);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator+=(value_type __other) && noexcept {
		__write(__read() + __other);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator*=(value_type __other) && noexcept {
		__write(__read() * __other);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator/=(value_type __other) && noexcept {
		__write(__read() / __other);
		return { _reference, _index };
	}

	raze_always_inline _Bit_reference operator%=(value_type __other) && noexcept {
		__write(__read() % __other);
		return { _reference, _index };
	}

	raze_always_inline friend void swap(
		_Bit_reference& __left,
		_Bit_reference& __right) noexcept
	{
		value_type __temp = static_cast<_Bit_reference&&>(__left);
		static_cast<_Bit_reference&&>(__left) = static_cast<value_type>(__right);
		static_cast<_Bit_reference&&>(__right) = std::move(__temp);
	}

	raze_always_inline friend void swap(
		_Bit_reference&	__left,
		value_type&		__right) noexcept
	{
		value_type __temp = static_cast<_Bit_reference&&>(__left);
		static_cast<_Bit_reference&&>(__left) = std::move(__right);
		__right = std::move(__temp);
	}

	raze_always_inline friend void swap(
		value_type&		__left,
		_Bit_reference&	__right) noexcept
	{
		value_type __temp = static_cast<_Bit_reference&&>(__right);
		static_cast<_Bit_reference&&>(__right) = std::move(__left);
		__left = std::move(__temp);
	}
private:
	raze_always_inline value_type __read() const noexcept {
		return math::__bit_test(_reference, _index);
	}

	raze_always_inline void __write(value_type __value) noexcept {
		if (__value)
			math::__bit_test_and_set(_reference, _index);
		else
			math::__bit_test_and_reset(_reference, _index);
	}

	_Type_& _reference;
	int32 _index = 0;
};

__RAZE_VX_NAMESPACE_END
