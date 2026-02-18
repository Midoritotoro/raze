#pragma once 

#include <raze/arch/CpuFeature.h>
#include <raze/compatibility/SimdCompatibility.h>

#include <src/raze/type_traits/IntegralProperties.h>
#include <src/raze/type_traits/TypeCheck.h>

#include <src/raze/datapar/SimdIntegralTypesCheck.h>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <arch::ISA _ISA_>
constexpr inline bool __is_generation_supported_v =
    arch::__contains<_ISA_, __ymm_features, __xmm_features, __zmm_features>::value;

template <typename _VectorElementType_>
constexpr inline bool __is_pointer_decay_v = std::is_pointer_v<std::decay_t<_VectorElementType_>>;

template <typename _VectorElementType_>
constexpr inline bool __is_vector_type_supported_v =
    std::is_arithmetic_v<std::decay_t<_VectorElementType_>> ||
    __is_pointer_decay_v<_VectorElementType_> || 
    std::is_same_v<std::decay_t<_VectorElementType_>, std::nullptr_t>;

template <>
constexpr inline bool __is_vector_type_supported_v<bool> = false;

template <
    arch::ISA   _ISA_,
    typename    _VectorElementType_,
    uint32      _Width_ = __default_width<_ISA_>>
struct __deduce_simd_vector_type__ {
private:
    using _Type_ = std::decay_t<_VectorElementType_>;

    static constexpr bool __is_fp64 = is_any_of_v<_Type_, double, long double> || (std::is_same_v<_Type_, std::nullptr_t> && sizeof(std::nullptr_t) == 8);
    static constexpr bool __is_fp32 = std::is_same_v<_Type_, float>;
    static constexpr bool __is_int  = is_nonbool_integral_v<_Type_> || (std::is_same_v<_Type_, std::nullptr_t> && sizeof(std::nullptr_t) == 4);
    static constexpr bool __is_ptr  = __is_pointer_decay_v<_VectorElementType_>;
    static constexpr bool __use_i   = __is_int || __is_ptr;

public:
    using type =
        std::conditional_t<
            _Width_ == 512,
                std::conditional_t<
                    __is_fp64, __m512d,
                    std::conditional_t<
                        __is_fp32, __m512,
                        std::conditional_t<
                            __use_i,   __m512i,
                                     void>>>,
        std::conditional_t<
            _Width_ == 256,
                std::conditional_t<
                    __is_fp64, __m256d,
                    std::conditional_t<
                        __is_fp32, __m256,
                        std::conditional_t<
                            __use_i,   __m256i,
                                     void>>>,
        std::conditional_t<
            _Width_ == 128,
                std::conditional_t<
                    __is_fp64, __m128d,
                    std::conditional_t<
                        __is_fp32, __m128,
                        std::conditional_t<
                            __use_i,   __m128i,
                                     void>>>,
        void>>>;
};

template <
    arch::ISA   _ISA_,
    typename    _VectorElementType_,
    uint32      _Width_>
using __deduce_simd_vector_type = typename __deduce_simd_vector_type__<_ISA_, _VectorElementType_, _Width_>::type;

template <sizetype size>
using __deduce_simd_shuffle_mask_type_helper = std::conditional_t<size <= 2, uint8,
		std::conditional_t<size <= 4, uint8,
			std::conditional_t<size <= 8, uint32,
				std::conditional_t<size <= 16, uint64, void>>>>;


template <sizetype _Size_>
using __deduce_simd_mask_type_helper = std::conditional_t<_Size_ <= 8, uint8,
		std::conditional_t<_Size_ <= 16, uint16,
			std::conditional_t<_Size_ <= 32, uint32,
				std::conditional_t<_Size_ <= 64, uint64, void>>>>;

template <
	arch::ISA	_ISA_,
	typename	_Element_,
    uint32      _Width_>
using __deduce_simd_mask_type = __deduce_simd_mask_type_helper<((_Width_ / 8) / sizeof(_Element_))>;

template <
	arch::ISA	_ISA_,
	typename	_Element_,
    uint32      _Width_>
using __deduce_simd_shuffle_mask_type = __deduce_simd_shuffle_mask_type_helper<(_Width_ / sizeof(_Element_))>;

template <arch::ISA _ISA_> 
constexpr bool __is_zeroupper_required_v =
    static_cast<int8>(_ISA_) == static_cast<int8>(arch::ISA::AVX2) ||
    static_cast<int8>(_ISA_) == static_cast<int8>(arch::ISA::AVX);

template <
    arch::ISA _SimdGenerationFirst_,
    arch::ISA _SimdGenerationSecond_>
constexpr bool __is_simd_feature_superior_v = (static_cast<uint8>(_SimdGenerationFirst_) > static_cast<uint8>(_SimdGenerationSecond_));

__RAZE_TYPE_TRAITS_NAMESPACE_END