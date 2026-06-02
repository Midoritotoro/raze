#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_, sizetype ... _Indices_>
struct _Shuffle_pattern {
	static_assert(sizeof...(_Indices_) > 0);
	using vector_type = _Simd_;

	constexpr _Shuffle_pattern() noexcept {}
	constexpr ~_Shuffle_pattern() noexcept {}

	constexpr _Shuffle_pattern(const _Shuffle_pattern&) noexcept {}
	constexpr _Shuffle_pattern& operator=(const _Shuffle_pattern&) noexcept {
		return *this;
	}

	static constexpr auto size() noexcept {
		return sizeof...(_Indices_);
	}

	static constexpr std::integer_sequence<size_t, _Indices_...> get() noexcept {
		return std::integer_sequence<size_t, _Indices_...>{};
	}

	template <sizetype _I_>
	static constexpr auto __at() noexcept {
		return std::get<_I_>(std::make_tuple(_Indices_...));
	}

	template <sizetype _I_>
	static constexpr std::integral_constant<sizetype, __at<_I_>()> at() noexcept {
		return {};
	}

	constexpr auto operator[](sizetype __i) const noexcept {
		return to_array<sizetype>()[__i];
	}

	template <std::unsigned_integral _IdxType_>
	static constexpr std::array<_IdxType_, size()> to_array() noexcept {
		return std::array<_IdxType_, size()> { static_cast<_IdxType_>(_Indices_)... };
	}
};

template <class _Pattern_>
using pattern_vector_t = typename _Pattern_::vector_type;

__RAZE_VX_NAMESPACE_END
