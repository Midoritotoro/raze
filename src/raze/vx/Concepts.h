#pragma once 

#include <src/raze/vx/hw/x86/mask/SimdMaskTypeCheck.h>
#include <raze/vx/Abi.h>
// #include <raze/vx/Mask.h>

__RAZE_VX_NAMESPACE_BEGIN

struct scalar_tag {};

template <class _Simd_>
concept simd_type = __is_valid_simd_v<_Simd_>;

template <class _Type_>
concept simd_or_arithmetic_type = simd_type<_Type_> || arithmetic_type<_Type_>;

template <class _Type_>
concept floating_point_simd = simd_type<_Type_> || std::floating_point<typename _Type_::value_type>;

template <class _Type_>
concept floating_point_simd_or_scalar_type = floating_point_simd<_Type_> || std::floating_point<_Type_>;

template <class _SimdMask_>
concept simd_mask_type = __is_simd_mask_v<_SimdMask_>;

template <class _SimdOrMask_>
concept simd_or_mask_type = simd_mask_type<_SimdOrMask_> || simd_type<_SimdOrMask_>;

template <class _AlignmentPolicy_>
concept alignment_policy_type = requires {
	{ std::remove_cvref_t<_AlignmentPolicy_>::__alignment } -> std::convertible_to<bool>;
};

template <simd_type _Simd_>
using abi_t = typename _Simd_::abi_type;

template <simd_type _Simd_>
constexpr auto __has_scalar_chunks_v = _Simd_::__has_scalar_chunks;

template <class ... _Types_>
concept has_any_scalar_chunks = (__has_scalar_chunks_v<_Types_> || ...);

template <class ... _Types_>
concept same_abi_isa = ((abi_t<_Types_>::isa == abi_t<std::tuple_element_t<0, std::tuple<_Types_...>>>::isa) && ...);

template <class _Simd_>
concept trivially_chunk_swappable = simd_type<_Simd_> && (_Simd_::size() != 0 && (_Simd_::size() & (_Simd_::size() - 1)) == 0);

template <class _Simd_>
concept native = simd_type<_Simd_> && _Simd_::is_native();

template <class _Index_>
concept index_simd_type = simd_type<_Index_> && std::is_unsigned_v<typename _Index_::value_type>;

template <class _Index_, class _Simd_>
concept index_type_for = simd_type<_Simd_> && index_simd_type<_Index_> && (_Simd_::size() == _Index_::size())
	&& (sizeof(typename _Index_::value_type) == sizeof(typename _Simd_::value_type));

template <simd_type _Simd_>
struct __zeroupper_at_destroy_guard {
	__zeroupper_at_destroy_guard() noexcept = default;
	__zeroupper_at_destroy_guard(const __zeroupper_at_destroy_guard&) noexcept = delete;
	__zeroupper_at_destroy_guard(__zeroupper_at_destroy_guard&&) noexcept = default;

	~__zeroupper_at_destroy_guard() noexcept {
		if constexpr (__has_avx2_support_v<abi_t<_Simd_>::isa>)
			_mm256_zeroupper();
	}

	__zeroupper_at_destroy_guard& operator=(const __zeroupper_at_destroy_guard&) noexcept = delete;
	__zeroupper_at_destroy_guard& operator=(__zeroupper_at_destroy_guard&&) noexcept = default;
};

template <simd_type _Simd_>
raze_nodiscard raze_always_inline __zeroupper_at_destroy_guard<_Simd_> make_guard() noexcept {
	return {};
}

__RAZE_VX_NAMESPACE_END
