#pragma once 

#include <src/raze/vx/hw/Cast.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class V> 
class _Simd_element_reference {
public:
	using value_type = typename _Simd_::value_type;

	simd_element_reference(_Simd_& v, i32 i) noexcept:
		_reference(v),
		_index(i)
	{
		raze_debug_assert_log(i >= 0 && i < _Simd_::size(), "Index out of range. ");
	}

	raze_always_inline operator value_type() const noexcept {
		return read();
	}

	raze_always_inline simd_element_reference operator=(value_type value) && noexcept {
		write(value);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator=(simd_element_reference&& ref) && noexcept {
		write(ref);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator=(const simd_element_reference& ref) && noexcept {
		write(ref);
		return { _reference, _index };
	}

	raze_always_inline value_type operator++(int) && noexcept {
		auto value = read();
		const auto old_value = value;
		write(++value);
		return old_value;
	}

	raze_always_inline simd_element_reference operator++() && noexcept {
		write(++read());
		return { _reference, _index };
	}

	raze_always_inline value_type operator--(int) && noexcept {
		auto value = read();
		const auto old_value = value;
		write(--value);
		return old_value;
	}

	raze_always_inline simd_element_reference operator--() && noexcept {
		write(--read());
		return { _reference, _index };
	}
	
	raze_always_inline simd_element_reference operator&=(value_type other) && noexcept {
		write(read() & other);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator|=(value_type other) && noexcept {
		write(read() | other);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator^=(value_type other) && noexcept {
		write(read() ^ other);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator>>=(i32 shift) && noexcept {
		write(read() >> shift);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator<<=(i32 shift) && noexcept {
		write(read() << shift);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator-=(value_type other) && noexcept {
		write(read() - other);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator+=(value_type other) && noexcept {
		write(read() + other);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator*=(value_type other) && noexcept {
		write(read() * other);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator/=(value_type other) && noexcept {
		write(read() / other);
		return { _reference, _index };
	}

	raze_always_inline simd_element_reference operator%=(value_type other) && noexcept {
		write(read() % other);
		return { _reference, _index };
	}

	raze_always_inline friend void swap(simd_element_reference& left,  simd_element_reference& right) noexcept {
		value_type temp = static_cast<simd_element_reference&&>(left);
		static_cast<simd_element_reference&&>(left) = static_cast<value_type>(right);
		static_cast<simd_element_reference&&>(right) = std::move(temp);
	}

	raze_always_inline friend void swap(simd_element_reference& left, value_type& right) noexcept {
		value_type temp = static_cast<simd_element_reference&&>(left);
		static_cast<simd_element_reference&&>(left) = std::move(right);
		right = std::move(temp);
	}

	raze_always_inline friend void swap(value_type&	left, simd_element_reference& right) noexcept {
		value_type temp = static_cast<simd_element_reference&&>(right);
		static_cast<simd_element_reference&&>(right) = std::move(left);
		left = std::move(temp);
	}
private:
	raze_always_inline value_type read() const noexcept {
		return _reference.__extract(_index);
	}
	
	raze_always_inline void write(value_type value) noexcept {
		_reference.__insert(_index, value);
	}

	_Simd_& _reference;
	i32 _index = 0;
};

__RAZE_VX_NAMESPACE_END
