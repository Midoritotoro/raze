#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Gen>
struct tail_mask {
	using mask_type = decltype(std::declval<Gen>()());
	tail_mask(sizetype tail_bytes, Gen&& gen) noexcept : _tail_bytes(tail_bytes), _gen(std::move(gen)) {}

	raze_always_inline operator mask_type() const noexcept {
		return _gen();
	}

	raze_always_inline mask_type operator()() const noexcept {
		return _gen();
	}

	raze_always_inline mask_type mask() const noexcept {
		return _gen();
	}

	raze_always_inline sizetype tail_bytes() const noexcept {
		return _tail_bytes;
	}

	raze_always_inline sizetype tail_elements() const noexcept {
		return _tail_bytes / sizeof(typename mask_type::value_type);
	}
private:
	Gen _gen;
	sizetype _tail_bytes;
};

template <class T>
concept tail_mask_type = requires(T const& ignore, sizetype bytes) {
	typename T::mask_type;

    { ignore.tail_bytes() } -> std::convertible_to<sizetype>;
    { ignore.tail_elements() } -> std::convertible_to<sizetype>;
	{ ignore.mask() } -> std::same_as<typename T::mask_type>;
    { ignore() } -> std::same_as<typename T::mask_type>;
	{ static_cast<typename T::mask_type>(ignore) } -> std::same_as<typename T::mask_type>;
};

__RAZE_ALGORITHM_NAMESPACE_END
