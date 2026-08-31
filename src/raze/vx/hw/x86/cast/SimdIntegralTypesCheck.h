#pragma once 

#include <raze/arch/CpuFeature.h>

#include <src/raze/traits/TypeCheck.h>
#include <src/raze/traits/IsVirtualBaseOf.h>
#include <src/raze/vx/hw/x86/mask/SimdMaskTypeCheck.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi = x86_abi<native_size<T>>>
class simd;

template <class T>
constexpr bool is_intrin_type_v = traits::is_any_of_v<std::remove_cvref_t<T>,
	__m128, __m128i, __m128d, __m256, __m256i, __m256d, __m512, __m512i, __m512d>;

template <class T>
concept intrin_type = is_intrin_type_v<T>;

template <class T>
concept arithmetic_type = std::is_arithmetic_v<T>;

template <class T>
concept raw_mask_type = intrin_type<T> || std::is_integral_v<T>;

template <class T>
concept intrin_or_arithmetic_type = intrin_type<T> || arithmetic_type<T>;

template <class T>
concept epi64 = ((std::is_signed_v<T> && !std::is_floating_point_v<T>) 
	|| std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t>) && sizeof(T) == 8;

template <class T>
concept epu64 = ((std::is_unsigned_v<T> && !std::is_floating_point_v<T>) 
	|| std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t>) && sizeof(T) == 8;

template <class T>
concept epi32 = ((std::is_signed_v<T> && !std::is_floating_point_v<T>) 
	|| std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t>) && sizeof(T) == 4;

template <class T>
concept epu32 = ((std::is_unsigned_v<T> && !std::is_floating_point_v<T>)
	|| std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t>) && sizeof(T) == 4;

template <class T>
concept epi16 = sizeof(T) == 2 && std::is_signed_v<T> && !std::is_floating_point_v<T>;

template <class T>
concept epu16 = sizeof(T) == 2 && std::is_unsigned_v<T> && !std::is_floating_point_v<T>;

template <class T>
concept epi8  = sizeof(T) == 1 && std::is_signed_v<T> && !std::is_floating_point_v<T>;

template <class T>
concept epu8  = sizeof(T) == 1 && std::is_unsigned_v<T> && !std::is_floating_point_v<T>;

template <class T>
concept pd = sizeof(T) == 8 && traits::is_any_of_v<T, f64, long double>;

template <class T>
concept ps = sizeof(T) == 4 && std::is_same_v<T, f32>;

template <class V, class = void>
struct is_valid_simd: 
	std::false_type
{};

template <class V>
struct is_valid_simd<
    V,
    std::void_t<simd<typename V::value_type,
        typename V::abi_type>>>
    : std::bool_constant<
        traits::is_virtual_base_of_v<
            simd<typename V::value_type,
                    typename V::abi_type>,
            V> ||
        std::is_same_v<
            simd<typename V::value_type,
                    typename V::abi_type>,
            V>> 
{};

template <class V>
constexpr bool is_valid_simd_v = is_valid_simd<std::remove_cvref_t<V>>::value;

template <arch::ISA ISA>
concept has_sse2 = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::SSE2);

template <arch::ISA ISA>
concept has_sse3 = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::SSE3);

template <arch::ISA ISA>
concept has_ssse3 = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::SSSE3);

template <arch::ISA ISA>
concept has_sse41 = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::SSE41);

template <arch::ISA ISA>
concept has_sse42 = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::SSE42);

template <arch::ISA ISA>
concept has_avx = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::AVX);

template <arch::ISA ISA>
concept has_fma3 = static_cast<int>(ISA) == static_cast<int>(arch::ISA::FMA3) 
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX2FMA3)
    || static_cast<int>(ISA) >= static_cast<int>(arch::ISA::AVX512F);

template <arch::ISA ISA>
concept has_avx2 = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::AVX2);

template <arch::ISA ISA> 
concept has_avx512f = static_cast<int>(ISA) >= static_cast<int>(arch::ISA::AVX512F);

template <arch::ISA ISA>
concept has_avx512vbmi = static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMIDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMIVLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMIVL) 
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2DQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2VLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2VL);

template <arch::ISA ISA>
concept has_avx512vbmi2 = static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2DQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2VLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2VL);

template <arch::ISA ISA>
concept has_avx512bw = static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512BW)
	|| static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512BWDQ)
	|| static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLBWDQ)
	|| static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLBW)
    || has_avx512vbmi<ISA> || has_avx512vbmi2<ISA>;

template <arch::ISA ISA>
concept has_avx512dq = static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512DQ)
	|| static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512BWDQ)
	|| static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLBWDQ)
	|| static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMIDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2DQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMIVLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2VLDQ);

template <arch::ISA ISA>
concept has_avx512vl = static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMIVLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2VLDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLF)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLBW)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VLBWDQ)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMIVL)
    || static_cast<int>(ISA) == static_cast<int>(arch::ISA::AVX512VBMI2VL);


#if defined(__BMI2__)
#  if defined(raze_cpp_clang)
#    if __has_feature(bmi2)
       template <arch::ISA ISA>
       concept has_bmi2 = true;
#    else
       template <arch::ISA ISA>
       concept has_bmi2 = false;
#    endif // __has_feature(bmi2)
#  else
    template <arch::ISA ISA>
    concept has_bmi2 = true;
#  endif
#else
#  if defined(raze_cpp_msvc_only)
    template <arch::ISA ISA>
    concept has_bmi2 = has_avx2<ISA>;
#  else 
    template <arch::ISA ISA>
    concept has_bmi2 = false;
#  endif // defined(raze_cpp_msvc_only)
#endif // defined(__BMI2__)

__RAZE_VX_NAMESPACE_END
