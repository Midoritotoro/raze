#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_, sizetype ... _Indices_>
struct _Shuffle_pattern {
	static_assert(sizeof...(_Indices_) > 0);
    // static_assert(sizeof...(_Indices_) == _Simd_::size());

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

	static constexpr std::integer_sequence<sizetype, _Indices_...> get() noexcept {
		return std::integer_sequence<sizetype, _Indices_...>{};
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
    raze_no_stack_protector raze_always_inline static auto as_native() noexcept {
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
            [] (sizetype __a, sizetype __b) constexpr noexcept { return __a >= __b; },
            std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _Divisor_>
    constexpr auto operator%(std::integral_constant<sizetype, _Divisor_> __divisor) const noexcept {
        return [=] <sizetype... __I>(std::index_sequence<__I...>) {
            return _Shuffle_pattern<_Simd_, (__at<__I>() % __divisor)...>{};
        } (std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _Divisor_>
    constexpr auto operator/(std::integral_constant<sizetype, _Divisor_> __divisor) const noexcept {
        return [=] <sizetype... __I>(std::index_sequence<__I...>) {
            return _Shuffle_pattern<_Simd_, (__at<__I>() / __divisor)...>{};
        } (std::make_index_sequence<sizeof...(_Indices_)>{});
    }

    template <sizetype _Offset_>
    constexpr auto offset(std::integral_constant<sizetype, _Offset_> __offset) const noexcept {
        return [=] <sizetype... __I>(std::index_sequence<__I...>) {
            return _Shuffle_pattern<_Simd_, (__at<__I + __offset>())...>{};
        } (std::make_index_sequence<size() - __offset>{});
    }

    constexpr auto crossing_lanes() const noexcept {
        constexpr sizetype __simd_size = sizeof(_Simd_);
        constexpr sizetype __lanes = (__simd_size >= 16) ? (__simd_size / 16) : 1;
        constexpr sizetype __L = size() / __lanes;

        return [] <sizetype... __I>(std::index_sequence<__I...>) {
            return _Shuffle_pattern<_Simd_, ((__I / __L) != (__at<__I>() / __L) ? static_cast<sizetype>(-1) : __at<__I>())...>{};
        }(std::make_index_sequence<size()>{});
    }

    constexpr auto non_crossing_lanes() const noexcept {
        constexpr sizetype __simd_size = sizeof(_Simd_);
        constexpr sizetype __lanes = (__simd_size >= 16) ? (__simd_size / 16) : 1;
        constexpr sizetype __L = size() / __lanes;

        return [] <sizetype... __I>(std::index_sequence<__I...>) {
            return _Shuffle_pattern<_Simd_, ((__I / __L) == (__at<__I>() / __L) ? static_cast<sizetype>(-1) : __at<__I>())...>{};
        }(std::make_index_sequence<size()>{});
    }

    template <sizetype... I>
    static constexpr auto __split_impl(std::index_sequence<I...>)
    {
        using HalfSimd = simd<typename _Simd_::value_type, resize_abi_t<abi_t<_Simd_>, size() / 2>>;

        return std::pair{
            [] <sizetype... J>(std::index_sequence<J...>) {
                return _Shuffle_pattern<HalfSimd, __at<J>()...>{};
            }(std::make_index_sequence<size() / 2>{}),

            []<sizetype... J>(std::index_sequence<J...>) {
                return _Shuffle_pattern<HalfSimd, __at<J + size() / 2>()...>{};
            }(std::make_index_sequence<size() / 2>{})
        };
    }

    constexpr auto split() const noexcept {
        return __split_impl(std::make_index_sequence<size()>{});
    }

    template <class _Predicate_>
    static constexpr auto to_mask(_Predicate_ __pred) noexcept {
        using mask_type = simd_mask<typename _Simd_::value_type, abi_t<_Simd_>>;
        alignas(64) static constexpr bool __mask[size()] { __pred(_Indices_)... };
        return mask_type(__mask, __aligned_policy{});
    }

    template <class _Predicate1_, class _Predicate2_>
    static constexpr auto to_mask(_Predicate1_ __low, _Predicate2_ __high) noexcept {
        using mask_type = simd_mask<typename _Simd_::value_type, abi_t<_Simd_>>;

        static constexpr std::array<sizetype, size()> __idx = { _Indices_... };
        alignas(64) static constexpr std::array<bool, size()> __mask = [=] {
            std::array<bool, size()> __m {};

            for (auto __i = 0; __i < size(); ++__i)
                __m[__i] = (__i < size() / 2) ? __low(__idx[__i]) : __high(__idx[__i]);

            return __m;
        }();

        return mask_type(__mask.data(), __aligned_policy{});
    }
};

template <class _Pattern_>
using pattern_vector_t = typename _Pattern_::vector_type;

__RAZE_VX_NAMESPACE_END
