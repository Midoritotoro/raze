#pragma once 

#include <src/raze/vx/hw/Cast.h>
#include <src/raze/utility/Assert.h>
#include <src/raze/vx/Concepts.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class Mask>
class simd_mask_reference {
	using element_type = typename Mask::value_type;
public:
	using value_type = bool;

	simd_mask_reference(Mask& mask, i32 i) noexcept :
		_reference(mask),
		_index(i)
	{
		raze_assert(i >= 0 && i < Mask::size());
	}

	raze_always_inline operator value_type() const noexcept {
		return read();
	}

	raze_always_inline simd_mask_reference operator=(value_type value) && noexcept {
		write(value);
		return { _reference, _index };
	}

	raze_always_inline value_type operator++(int) && noexcept {
		auto value = read();
		const auto old_value = value;
		write(++value);
		return old_value;
	}

	raze_always_inline simd_mask_reference operator++() && noexcept {
		write(++read());
		return { _reference, _index };
	}

	raze_always_inline value_type operator--(int) && noexcept {
		auto value = read();
		const auto old_value = value;
		write(--value);
		return old_value;
	}

	raze_always_inline simd_mask_reference operator--() && noexcept {
		write(--read());
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator&=(value_type other) && noexcept {
		write(read() & other);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator|=(value_type other) && noexcept {
		write(read() | other);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator^=(value_type other) && noexcept {
		write(read() ^ other);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator>>=(i32 shift) && noexcept {
		write(read() >> shift);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator<<=(i32 shift) && noexcept {
		write(read() << shift);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator-=(value_type other) && noexcept {
		write(read() - other);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator+=(value_type other) && noexcept {
		write(read() + other);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator*=(value_type other) && noexcept {
		write(read() * other);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator/=(value_type other) && noexcept {
		write(read() / other);
		return { _reference, _index };
	}

	raze_always_inline simd_mask_reference operator%=(value_type other) && noexcept {
		write(read() % other);
		return { _reference, _index };
	}

	raze_always_inline friend void swap(simd_mask_reference& left, simd_mask_reference& right) noexcept {
		value_type temp = static_cast<simd_mask_reference&&>(left);
		static_cast<simd_mask_reference&&>(left) = static_cast<value_type>(right);
		static_cast<simd_mask_reference&&>(right) = std::move(temp);
	}

	raze_always_inline friend void swap(simd_mask_reference& left, value_type& right) noexcept {
		value_type temp = static_cast<simd_mask_reference&&>(left);
		static_cast<simd_mask_reference&&>(left) = std::move(right);
		right = std::move(temp);
	}

	raze_always_inline friend void swap(value_type& left, simd_mask_reference& right) noexcept {
		value_type temp = static_cast<simd_mask_reference&&>(right);
		static_cast<simd_mask_reference&&>(right) = std::move(left);
		left = std::move(temp);
	}
private:
	raze_always_inline value_type read() const noexcept {
		return _reference.__extract(_index);
	}

	raze_always_inline void write(value_type value) noexcept {
		_reference.__insert(_index, value);
	}

	Mask& _reference;
	i32 _index = 0;
};

__RAZE_VX_NAMESPACE_END
