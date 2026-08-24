#pragma once 

#include <src/raze/vx/hw/x86/mask/SimdMaskTypeCheck.h>
#include <raze/vx/Abi.h>

__RAZE_VX_NAMESPACE_BEGIN

struct scalar_tag {};

template <class T>
concept simd_type = __is_valid_simd_v<T>;

template <simd_type T>
struct tail_tag {
	using original_type = T;
};

template <class T>
concept simd_or_arithmetic_type = simd_type<T> || arithmetic_type<T>;

template <class T>
concept floating_point_simd = simd_type<T> || std::floating_point<typename T::value_type>;

template <class T>
concept floating_point_simd_or_scalar_type = floating_point_simd<T> || std::floating_point<T>;

template <class T>
concept simd_mask_type = __is_simd_mask_v<T>;

template <class T>
concept simd_or_mask_type = simd_mask_type<T> || simd_type<T>;

template <class Policy>
concept alignment_policy_type = requires {
	{ std::remove_cvref_t<Policy>::alignment } -> std::convertible_to<bool>;
};

template <simd_type V>
using abi_t = typename V::abi_type;

template <simd_type V>
constexpr auto has_scalar_chunks_v = _Simd_::__has_scalar_chunks;

template <class ... Ts>
concept has_any_scalar_chunks = (has_scalar_chunks_v<Ts> || ...);

template <class ... Ts>
concept same_abi_isa = ((abi_t<Ts>::isa == abi_t<std::tuple_element_t<0, std::tuple<Ts...>>>::isa) && ...);

template <class T>
concept trivially_chunk_swappable = simd_type<T> && (T::size() != 0 && (T::size() & (T::size() - 1)) == 0);

template <class T>
concept native = simd_type<T> && T::is_native();

template <class T>
concept index_simd_type = simd_type<T> && std::is_unsigned_v<typename T::value_type>;

template <class Idx, class T>
concept index_type_for = simd_type<T> && index_simd_type<Idx> && (T::size() == Idx::size())
	&& (sizeof(typename Idx::value_type) == sizeof(typename T::value_type));

template <simd_type V>
struct zeroupper_at_destroy_guard {
	zeroupper_at_destroy_guard() noexcept = default;
	zeroupper_at_destroy_guard(const zeroupper_at_destroy_guard&) noexcept = delete;
	zeroupper_at_destroy_guard(zeroupper_at_destroy_guard&&) noexcept = default;

	~zeroupper_at_destroy_guard() noexcept {
		if constexpr (has_avx<abi_t<V>::isa>)
			_mm256_zeroupper();
	}

	zeroupper_at_destroy_guard& operator=(const zeroupper_at_destroy_guard&) noexcept = delete;
	zeroupper_at_destroy_guard& operator=(zeroupper_at_destroy_guard&&) noexcept = default;
};

template <simd_type V>
raze_nodiscard raze_always_inline zeroupper_at_destroy_guard<V> make_guard() noexcept {
	return {};
}

__RAZE_VX_NAMESPACE_END
