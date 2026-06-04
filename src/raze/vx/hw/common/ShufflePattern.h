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

	template <intrin_type _Intrin_>
	raze_always_inline static auto as_native() noexcept {
		using _IdxType = typename IntegerForSize<sizeof(_Intrin_) / size()>::Unsigned;
		alignas(sizeof(_Intrin_)) static constexpr _IdxType __idx[size()] { _Indices_ ... };
		return _Load<abi_t<_Simd_>::isa, _Intrin_>()(__idx, __aligned_policy{});
	}

	template <std::unsigned_integral _From_, std::unsigned_integral _To_>
	static constexpr auto expand() noexcept
		requires(sizeof(_From_) >= sizeof(_To_))
	{
		constexpr auto __n = sizeof(_From_) / sizeof(_To_);
		constexpr auto __len = sizeof...(_Indices_);

		return [] <sizetype... __I>(std::index_sequence<__I...>) {
			return _Shuffle_pattern<_Simd_, ((__at<__I / __n>() * __n) + (__I % __n))...>{};
		}(std::make_index_sequence<__len * __n>{});
	}

	static constexpr auto widen() noexcept
		requires((sizeof...(_Indices_) % 2) == 0)
	{
		constexpr auto __new_len = sizeof...(_Indices_) / 2;

		return [] <sizetype... __I>(std::index_sequence<__I...>) {
			return _Shuffle_pattern<_Simd_, (__at<__I * 2>() / 2)...>{};
		}(std::make_index_sequence<__new_len>{});
	}
	
    template <sizetype _I_, sizetype... __Is_>
	raze_always_inline static constexpr sizetype __make_mask(
        bool (__pred)(sizetype, sizetype),
        std::index_sequence<__Is_...>) noexcept
    {
        return ((__pred(_Indices_, _I_) ? (sizetype{1} << __Is_) : sizetype{0}) | ...);
    }

    template <sizetype _I_>
	raze_always_inline constexpr sizetype operator==(std::integral_constant<sizetype, _I_>) const noexcept {
        return __make_mask<_I_>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a == __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _I_>
	raze_always_inline constexpr sizetype operator!=(std::integral_constant<sizetype, _I_>) const noexcept {
        return __make_mask<_I_>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a != __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _I_>
	raze_always_inline constexpr sizetype operator<(std::integral_constant<sizetype, _I_>) const noexcept {
        return __make_mask<_I_>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a < __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _I_>
	raze_always_inline constexpr sizetype operator<=(std::integral_constant<sizetype, _I_>) const noexcept {
        return __make_mask<_I_>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a <= __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _I_>
	raze_always_inline constexpr sizetype operator>(std::integral_constant<sizetype, _I_>) const noexcept {
        return __make_mask<_I_>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a > __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _I_>
	raze_always_inline constexpr sizetype operator>=(std::integral_constant<sizetype, _I_>) const noexcept {
        return __make_mask<_I_>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a >= __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

	    template <sizetype... _OtherIndices_, sizetype... __Is_>
    raze_always_inline static constexpr sizetype __make_mask2(
        bool (__pred)(sizetype, sizetype),
        std::index_sequence<__Is_...>) noexcept
    {
        constexpr sizetype __this_arr[] = { _Indices_ ... };
        constexpr sizetype __other_arr[] = { _OtherIndices_ ... };
        return ((__pred(__this_arr[__Is_], __other_arr[__Is_]) ? (sizetype{1} << __Is_) : sizetype{0}) | ...);
    }

    template <simd_type _OtherSimd_, sizetype... _OtherIndices_>
    raze_always_inline constexpr sizetype operator==(const _Shuffle_pattern<_OtherSimd_, _OtherIndices_...>&) const noexcept
        requires(sizeof...(_Indices_) == sizeof...(_OtherIndices_))
    {
        return __make_mask2<_OtherIndices_...>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a == __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <simd_type _OtherSimd_, sizetype... _OtherIndices_>
    raze_always_inline constexpr sizetype operator!=(const _Shuffle_pattern<_OtherSimd_, _OtherIndices_...>&) const noexcept
        requires(sizeof...(_Indices_) == sizeof...(_OtherIndices_))
    {
        return __make_mask2<_OtherIndices_...>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a != __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <simd_type _OtherSimd_, sizetype... _OtherIndices_>
    raze_always_inline constexpr sizetype operator<(const _Shuffle_pattern<_OtherSimd_, _OtherIndices_...>&) const noexcept
        requires(sizeof...(_Indices_) == sizeof...(_OtherIndices_))
    {
        return __make_mask2<_OtherIndices_...>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a < __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <simd_type _OtherSimd_, sizetype... _OtherIndices_>
    raze_always_inline constexpr sizetype operator<=(const _Shuffle_pattern<_OtherSimd_, _OtherIndices_...>&) const noexcept
        requires(sizeof...(_Indices_) == sizeof...(_OtherIndices_))
    {
        return __make_mask2<_OtherIndices_...>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a <= __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <simd_type _OtherSimd_, sizetype... _OtherIndices_>
    raze_always_inline constexpr sizetype operator>(const _Shuffle_pattern<_OtherSimd_, _OtherIndices_...>&) const noexcept
        requires(sizeof...(_Indices_) == sizeof...(_OtherIndices_))
    {
        return __make_mask2<_OtherIndices_...>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a > __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <simd_type _OtherSimd_, sizetype... _OtherIndices_>
    raze_always_inline constexpr sizetype operator>=(const _Shuffle_pattern<_OtherSimd_, _OtherIndices_...>&) const noexcept
        requires(sizeof...(_Indices_) == sizeof...(_OtherIndices_))
    {
        return __make_mask2<_OtherIndices_...>(
            [](sizetype __a, sizetype __b) constexpr noexcept { return __a >= __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }
};

template <class _Pattern_>
using pattern_vector_t = typename _Pattern_::vector_type;

__RAZE_VX_NAMESPACE_END
