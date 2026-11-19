#pragma once 

#include <src/raze/traits/TypeCheck.h>
#include <src/raze/traits/IsVirtualBaseOf.h>
#include <raze/vx/Config.h>
#include <immintrin.h>

__RAZE_VX_NAMESPACE_BEGIN

template <sizetype N>
struct fixed_abi;

#if !defined(raze_cpp_nvhpc) || RAZE_HAS_AVX512F_SUPPORT
  using raze_mmask8_t = __mmask8;
  using raze_mmask16_t = __mmask16;
#else
  using raze_mmask8_t = u8;
  using raze_mmask16_t = u16;
#endif

#if !defined(raze_cpp_nvhpc) || RAZE_HAS_AVX512BW_SUPPORT
  using raze_mmask32_t = __mmask32;
  using raze_mmask64_t = __mmask64;
#else
  using raze_mmask32_t = u32;
  using raze_mmask64_t = u64;
#endif

template <u64 N>
using mmask_for_elements_helper = std::conditional_t<N <= 8, raze_mmask8_t,
    std::conditional_t<N <= 16, raze_mmask16_t,
    std::conditional_t<N <= 32, raze_mmask32_t,
    std::conditional_t<N <= 64, raze_mmask64_t, void>>>>;

template <u64 N>
using mmask_for_elements_t = mmask_for_elements_helper<N>;

template <arch::ISA ISA>
constexpr auto default_width = arch::is_xmm_v<ISA> ? 128 : arch::is_ymm_v<ISA> ? 256 : arch::is_zmm_v<ISA> ? 512 : -1;

template <class T>
constexpr auto native_size = (default_width<target_isa()> / 8) / sizeof(T);

template <arch::ISA ISA>
constexpr auto vector_default_size = default_width<ISA>;

template <class T, class Abi = fixed_abi<native_size<T>>>
class simd_mask;

template <class M, class = void>
struct is_simd_mask :
    std::false_type
{};

template <class M>
struct is_simd_mask<
    M,
    std::void_t<simd_mask<typename M::element_type,
    typename M::abi_type>>>
    : std::bool_constant<
    traits::is_virtual_base_of_v<
    simd_mask<typename M::element_type,
    typename M::abi_type>,
    M> ||
    std::is_same_v<simd_mask<typename M::element_type,
    typename M::abi_type>,
    M>>
{};

template <class M>
constexpr bool is_simd_mask_v = is_simd_mask<std::remove_cvref_t<M>>::value;

__RAZE_VX_NAMESPACE_END
