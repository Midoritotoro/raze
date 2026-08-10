#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Gen_>
struct tail_mask {
	using mask_type = decltype(std::declval<_Gen_>()());
	tail_mask(sizetype __tail_bytes, _Gen_&& __gen) noexcept : _tail_bytes(__tail_bytes), _gen(std::move(__gen)) {}

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
	_Gen_ _gen;
	sizetype _tail_bytes;
};

template <class _Tp_>
concept tail_mask_type = requires(_Tp_ const& __ignore, sizetype __bytes) {
	typename _Tp_::mask_type;

    { __ignore.tail_bytes() } -> std::convertible_to<sizetype>;
    { __ignore.tail_elements() } -> std::convertible_to<sizetype>;
	{ __ignore.mask() } -> std::same_as<typename _Tp_::mask_type>;
    { __ignore() } -> std::same_as<typename _Tp_::mask_type>;
	{ static_cast<typename _Tp_::mask_type>(__ignore) } -> std::same_as<typename _Tp_::mask_type>;
};

__RAZE_ALGORITHM_NAMESPACE_END
