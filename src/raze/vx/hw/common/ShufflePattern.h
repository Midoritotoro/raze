#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/vx/Concepts.h>
#include <src/raze/vx/hw/configurable/shape/SplitBy.h>


__RAZE_VX_NAMESPACE_BEGIN

template <simd_type V, sizetype ... Idxs>
struct shuffle_pattern;

template <class Pattern, sizetype ChunkSize, sizetype ChunkIndex, class Seq>
struct split_part_impl;

template <class Pattern, sizetype ChunkSize, sizetype ChunkIndex, sizetype... I>
struct split_part_impl<Pattern, ChunkSize, ChunkIndex, std::index_sequence<I...>> {
    using simd_type = typename Pattern::vector_type;
    using type = shuffle_pattern<split_by_simd_t<ChunkSize, simd_type, ChunkIndex>,
        Pattern::template __at<ChunkIndex * ChunkSize + I>()...>;
};

template <class Pattern, sizetype ChunkSize, sizetype ChunkIndex>
using split_part_t = typename split_part_impl<Pattern,
    ChunkSize, ChunkIndex, std::make_index_sequence<
        ((ChunkIndex + 1) * ChunkSize <= Pattern::size())
            ? ChunkSize : (Pattern::size() - ChunkIndex * ChunkSize)>>::type;

template <simd_type V, sizetype ... Idxs>
struct shuffle_pattern {
	static_assert(sizeof...(Idxs) > 0);

    static constexpr std::array<sizetype, sizeof...(Idxs)> idxs = { Idxs ... };

	using vector_type = V;

	constexpr shuffle_pattern() noexcept {}
	constexpr ~shuffle_pattern() noexcept {}

	constexpr shuffle_pattern(const shuffle_pattern&) noexcept {}
	constexpr shuffle_pattern& operator=(const shuffle_pattern&) noexcept {
		return *this;
	}

	static constexpr auto size() noexcept {
		return sizeof...(Idxs);
	}

	static constexpr std::integer_sequence<sizetype, Idxs...> get() noexcept {
		return std::integer_sequence<sizetype, Idxs...>{};
	}

	template <sizetype I>
    static constexpr auto __at() noexcept {
        return idxs[I];
	}

	template <sizetype I>
	static constexpr std::integral_constant<sizetype, __at<I>()> at() noexcept {
		return {};
	}

	constexpr auto operator[](sizetype i) const noexcept {
		return idxs[i];
	}

	template <std::unsigned_integral Idx>
	static constexpr std::array<Idx, size()> to_array() noexcept {
		return std::array<Idx, size()> { static_cast<Idx>(Idxs)... };
	}

	template <intrin_type Intrin>
    raze_no_stack_protector raze_always_inline static auto as_native() noexcept requires((sizeof(Intrin) / size()) != 0) {
		using Idx = typename IntegerForSize<sizeof(Intrin) / size()>::Unsigned;
		alignas(sizeof(Intrin)) static constexpr Idx idx[size()] { Idx(Idxs)... };
		return load_<abi_t<V>::isa, Intrin>(idx, aligned_policy{});
	}

	template <std::unsigned_integral From, std::unsigned_integral To>
	static constexpr auto expand() noexcept
		requires(sizeof(From) >= sizeof(To))
	{
		constexpr auto n = sizeof(From) / sizeof(To);
		return [] <sizetype... I> (std::index_sequence<I...>) {
			return shuffle_pattern<V, ((at<I / n>() * n) + (I % n))...>{};
		}(std::make_index_sequence<size() * n>{});
	}

	static constexpr auto widen() noexcept
		requires((size() % 2) == 0)
	{
		constexpr auto new_len = size() / 2;
		return [] <sizetype... I> (std::index_sequence<I...>) {
			return shuffle_pattern<V, (at<I * 2>() / 2)...>{};
		}(std::make_index_sequence<new_len>{});
	}
	
    template <sizetype I, sizetype... Is>
	raze_always_inline static constexpr sizetype make_mask(
        bool (pred)(sizetype, sizetype),
        std::index_sequence<Is...>) noexcept
    {
        return ((pred(Idxs, I) ? (sizetype{1} << Is) : sizetype{0}) | ...);
    }

    template <sizetype I>
	raze_always_inline constexpr sizetype operator==(std::integral_constant<sizetype, I>) const noexcept {
        return make_mask<I>(
            [](sizetype a, sizetype b) constexpr noexcept { return a == b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype I>
	raze_always_inline constexpr sizetype operator!=(std::integral_constant<sizetype, I>) const noexcept {
        return make_mask<I>(
            [](sizetype a, sizetype b) constexpr noexcept { return a != b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype I>
	raze_always_inline constexpr sizetype operator<(std::integral_constant<sizetype, I>) const noexcept {
        return make_mask<I>(
            [](sizetype a, sizetype b) constexpr noexcept { return a < b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype I>
	raze_always_inline constexpr sizetype operator<=(std::integral_constant<sizetype, I>) const noexcept {
        return make_mask<I>(
            [](sizetype a, sizetype b) constexpr noexcept { return a <= b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype I>
	raze_always_inline constexpr sizetype operator>(std::integral_constant<sizetype, I>) const noexcept {
        return make_mask<I>(
            [](sizetype a, sizetype b) constexpr noexcept { return a > b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype I>
	raze_always_inline constexpr sizetype operator>=(std::integral_constant<sizetype, I>) const noexcept {
        return make_mask<I>(
            [](sizetype a, sizetype b) constexpr noexcept { return a >= b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

	template <sizetype... OtherIdxs, sizetype... Is>
    raze_always_inline static constexpr sizetype make_mask2(
        bool (pred)(sizetype, sizetype),
        std::index_sequence<Is...>) noexcept
    {
        constexpr sizetype this_arr[] = { Idxs ... };
        constexpr sizetype other_arr[] = { OtherIdxs ... };
        return ((pred(this_arr[Is], other_arr[Is]) ? (sizetype{1} << Is) : sizetype{0}) | ...);
    }

    template <simd_type OtherV, sizetype... OtherIdxs>
    raze_always_inline constexpr sizetype operator==(const shuffle_pattern<OtherV, OtherIdxs...>&) const noexcept
        requires(sizeof...(Idxs) == sizeof...(OtherIdxs))
    {
        return make_mask2<OtherIdxs...>(
            [](sizetype a, sizetype b) constexpr noexcept { return a == b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <simd_type OtherV, sizetype... OtherIdxs>
    raze_always_inline constexpr sizetype operator!=(const shuffle_pattern<OtherV, OtherIdxs...>&) const noexcept
        requires(sizeof...(Idxs) == sizeof...(OtherIdxs))
    {
        return make_mask2<OtherIdxs...>(
            [](sizetype a, sizetype b) constexpr noexcept { return a != b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <simd_type OtherV, sizetype... OtherIdxs>
    raze_always_inline constexpr sizetype operator<(const shuffle_pattern<OtherV, OtherIdxs...>&) const noexcept
        requires(sizeof...(Idxs) == sizeof...(OtherIdxs))
    {
        return make_mask2<OtherIdxs...>(
            [](sizetype a, sizetype b) constexpr noexcept { return a < b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <simd_type OtherV, sizetype... OtherIdxs>
    raze_always_inline constexpr sizetype operator<=(const shuffle_pattern<OtherV, OtherIdxs...>&) const noexcept
        requires(sizeof...(Idxs) == sizeof...(OtherIdxs))
    {
        return make_mask2<OtherIdxs...>(
            [](sizetype a, sizetype b) constexpr noexcept { return a <= b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <simd_type OtherV, sizetype... OtherIdxs>
    raze_always_inline constexpr sizetype operator>(const shuffle_pattern<OtherV, OtherIdxs...>&) const noexcept
        requires(sizeof...(Idxs) == sizeof...(OtherIdxs))
    {
        return make_mask2<OtherIdxs...>(
            [](sizetype a, sizetype b) constexpr noexcept { return a > b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <simd_type OtherV, sizetype... OtherIdxs>
    raze_always_inline constexpr sizetype operator>=(const shuffle_pattern<OtherV, OtherIdxs...>&) const noexcept
        requires(sizeof...(Idxs) == sizeof...(OtherIdxs))
    {
        return make_mask2<OtherIdxs...>(
            [] (sizetype a, sizetype b) constexpr noexcept { return a >= b; },
            std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype Divisor>
    constexpr auto operator%(std::integral_constant<sizetype, Divisor> divisor) const noexcept {
        return [=] <sizetype... I> (std::index_sequence<I...>) {
            return shuffle_pattern<V, (at<I>() % divisor)...>{};
        } (std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype Divisor>
    constexpr auto operator/(std::integral_constant<sizetype, Divisor> divisor) const noexcept {
        return [=] <sizetype... I>(std::index_sequence<I...>) {
            return shuffle_pattern<V, (at<I>() / divisor)...>{};
        } (std::make_index_sequence<sizeof...(Idxs)>{});
    }

    template <sizetype Offset>
    constexpr auto offset(std::integral_constant<sizetype, Offset> offset) const noexcept {
        return [=] <sizetype... I>(std::index_sequence<I...>) {
            return shuffle_pattern<V, (__at<I + offset>())...>{};
        } (std::make_index_sequence<size() - offset>{});
    }

    constexpr auto crossing_lanes() const noexcept {
        constexpr sizetype simd_size = sizeof(V);
        constexpr sizetype lanes = (simd_size >= 16) ? (simd_size / 16) : 1;
        constexpr sizetype L = size() / lanes;

        return [] <sizetype... I>(std::index_sequence<I...>) {
            return shuffle_pattern<V, ((I / L) != (at<I>() / L) ? static_cast<sizetype>(-1) : at<I>())...>{};
        }(std::make_index_sequence<size()>{});
    }

    constexpr auto non_crossing_lanes() const noexcept {
        constexpr sizetype simd_size = sizeof(V);
        constexpr sizetype lanes = (simd_size >= 16) ? (simd_size / 16) : 1;
        constexpr sizetype L = size() / lanes;

        return [] <sizetype... I>(std::index_sequence<I...>) {
            return shuffle_pattern<V, ((I / L) == (at<I>() / L) ? static_cast<sizetype>(-1) : at<I>())...>{};
        }(std::make_index_sequence<size()>{});
    }

    template <sizetype... I>
    static constexpr auto split_impl(std::index_sequence<I...>) {
        using HalfSimd = simd<typename V::value_type, resize_abi_t<abi_t<V>, size() / 2>>;

        return std::pair{
            [] <sizetype... J> (std::index_sequence<J...>) {
                return shuffle_pattern<HalfSimd, at<J>()...>{};
            }(std::make_index_sequence<size() / 2>{}),

            [] <sizetype... J> (std::index_sequence<J...>) {
                return shuffle_pattern<HalfSimd, at<J + size() / 2>()...>{};
            }(std::make_index_sequence<size() / 2>{})
        };
    }

    constexpr auto split() const noexcept {
        return split_impl(std::make_index_sequence<size()>{});
    }

    template <sizetype N, sizetype... I>
    constexpr auto split_by_impl(std::index_sequence<I...>) const noexcept {
        constexpr sizetype chunk_size = (size() + N - 1) / N;
        return std::tuple<split_part_t<shuffle_pattern, chunk_size, I>...>{};
    }

    template <sizetype Chunks>
    consteval auto split_by() const noexcept {
        static_assert(Chunks > 0);
        return split_by_impl<Chunks>(std::make_index_sequence<Chunks>{});
    }

    template <class Pred>
    raze_no_stack_protector raze_always_inline static auto to_mask(Pred pred) noexcept {
        alignas(64) static constexpr bool mask[size()] { pred(Idxs)... };
        return typename V::mask_type(mask, aligned_policy{});
    }

    template <class Pred1, class Pred2>
    raze_no_stack_protector raze_always_inline static auto to_mask(Pred1 low, Pred2 high) noexcept {
        static constexpr std::array<sizetype, size()> idx = { Idxs... };
        alignas(64) static constexpr std::array<bool, size()> mask = [=] {
            std::array<bool, size()> m {};

            for (auto i = 0; i < size(); ++i)
                m[i] = (i < size() / 2) ? low(idx[i]) : high(idx[i]);

            return m;
        }();

        return typename V::mask_type(mask.data(), aligned_policy{});
    }

    raze_always_inline constexpr auto invalidate_cross_lane() const noexcept {
        constexpr sizetype simd_size = sizeof(V);
        constexpr sizetype lanes = (simd_size >= 16) ? (simd_size / 16) : 1;
        constexpr sizetype lane_size = size() / lanes;

        return [] <sizetype... I> (std::index_sequence<I...>) {
            return shuffle_pattern<V, (((at<I>() / lane_size) != (I / lane_size)) ? static_cast<sizetype>(-1) : at<I>())...>{};
        }(std::make_index_sequence<size()>{});
    }

#if defined(raze_cpp_clang) || defined(raze_cpp_gnu)
    template <intrin_type Intrin>
    raze_always_inline static auto builtin_shufflevector(Intrin v, Intrin v2) noexcept {
        using Elem = typename IntegerForSize<sizeof(Intrin) / size()>::Unsigned;
        using ExtVec = Elem __attribute__((vector_size(sizeof(Intrin))));

        ExtVec ext = __builtin_bit_cast(ExtVec, v);
        ExtVec ext2 = __builtin_bit_cast(ExtVec, v2);
        return __builtin_shufflevector(ext, ext2, Idxs...);
    }

    template <intrin_type Intrin>
    raze_always_inline static auto builtin_shufflevector(Intrin v) noexcept {
        using Elem = typename IntegerForSize<sizeof(Intrin) / size()>::Unsigned;
        using ExtVec = Elem __attribute__((vector_size(sizeof(Intrin))));

        ExtVec ext = __builtin_bit_cast(ExtVec, v);
        return __builtin_shufflevector(ext, ext, Idxs...);
    }
#endif
};

template <class Pattern>
using pattern_vector_t = typename Pattern::vector_type;

__RAZE_VX_NAMESPACE_END
