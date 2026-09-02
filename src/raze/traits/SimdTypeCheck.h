#pragma once 

#include <raze/arch/CpuFeature.h>
#include <raze/compatibility/SimdCompatibility.h>

#include <src/raze/traits/IntegralProperties.h>
#include <src/raze/traits/TypeCheck.h>

#include <src/raze/vx/hw/Cast.h>


__RAZE_TRAITS_NAMESPACE_BEGIN

template <arch::ISA ISA>
constexpr inline bool is_generation_supported_v =
    arch::contains<ISA, __ymm_features, __xmm_features, __zmm_features>::value;

template <class T>
constexpr inline bool is_pointer_decay_v = std::is_pointer_v<std::decay_t<T>>;

template <class T>
constexpr inline bool is_vector_type_supported_v = std::is_arithmetic_v<std::decay_t<T>> ||
    is_pointer_decay_v<T> || std::is_same_v<std::decay_t<T>, std::nullptr_t>;

template <>
constexpr inline bool is_vector_type_supported_v<bool> = false;

template <class T, u32 Width>
struct deduce_simd_vector_type_impl {
private:
    using Type = std::decay_t<T>;

    static constexpr bool is_fp64 = is_any_of_v<Type, f64, long double>;
    static constexpr bool is_fp32 = std::is_same_v<Type, f32>;
    static constexpr bool use_i = is_nonbool_integral_v<Type> || is_pointer_decay_v<T> || std::is_same_v<Type, std::nullptr_t>;
public:
    using type =
        std::conditional_t<
            Width == 512,
                std::conditional_t<
                    is_fp64, __m512d,
                    std::conditional_t<
                        is_fp32, __m512,
                        std::conditional_t<
                            use_i,   __m512i,
                                     void>>>,
        std::conditional_t<
            Width == 256,
                std::conditional_t<
                    is_fp64, __m256d,
                    std::conditional_t<
                        is_fp32, __m256,
                        std::conditional_t<
                            use_i,   __m256i,
                                     void>>>,
        std::conditional_t<
            Width == 128,
                std::conditional_t<
                    is_fp64, __m128d,
                    std::conditional_t<
                        is_fp32, __m128,
                        std::conditional_t<
                            use_i,   __m128i,
                                     void>>>,
        void>>>;
};

template <class T, u32 Width>
using deduce_simd_vector_type = typename deduce_simd_vector_type_impl<T, Width>::type;

__RAZE_TRAITS_NAMESPACE_END