#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/compact/CompressTables.h>

#include <src/raze/vx/hw/x86/mask/ToMask.h>
#include <src/raze/vx/hw/x86/mask/ToVector.h>

#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/algorithm/AdvanceBytes.h>

#include <src/raze/type_traits/SimdTypeCheck.h>
#include <raze/math/BitMath.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	uint32		_Width_,
	class		_Type_>
struct _Compress;

template <class _Type_>
struct _Compress<arch::ISA::SSE2, 128, _Type_> {
    template <
        class _IntrinType_,
        class _MaskType_>
	raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
		_IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
	{
		return (*this)(__vector, _To_mask<arch::ISA::SSE2, 128, _Type_>()(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
	{
		if constexpr (__is_epi64_v<_Type_> || __is_epu64_v<_Type_> || __is_pd_v<_Type_>) {
            switch ((__mask & 0x3)) {
                case 0:
                    return { 16, __vector };
                case 1:
                    __vector = __as<_IntrinType_>(_mm_shuffle_pd(
                        __as<__m128d>(__vector), __as<__m128d>(__vector), 0x3));
                    return { 8, __vector };

                case 2:
                    return { 8, __vector };

                default:
                    raze_assert_unreachable();
                    break;
            }
        }
        else if constexpr (__is_epi32_v<_Type_> || __is_epu32_v<_Type_> || __is_ps_v<_Type_>) {
            switch ((__mask & 0xF)) {
                case 0x0:
                    return { 16, __vector };
                case 0x1:
                    __vector = __as<_IntrinType_>(
                        _mm_shuffle_ps(__as<__m128>(__vector), __as<__m128>(__vector), 0xF9));
                    return { 12, __vector };
                case 0x2:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xF8));
                    return { 12, __vector };
                case 0x3:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xEE));
                    return { 8, __vector };
                case 0x4:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xF4));
                    return { 12, __vector };
                case 0x5:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xED));
                    return { 8, __vector };
                case 0x6:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xEC));
                
                    return { 8, __vector };
                case 0x7:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xE7));
                
                    return { 4, __vector };
                case 0x8:
                    return { 12, __vector };
                case 0x9:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xE9));
                    return { 8, __vector };
                case 0xA:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xE8));
                    return { 8, __vector };
                case 0xB:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xE6));
                    return { 4, __vector };
                case 0xC:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xE5));

                    return { 12, __vector };
                case 0xD:
                    __vector = __as<_IntrinType_>(_mm_shuffle_ps(
                        __as<__m128>(__vector), __as<__m128>(__vector), 0xE5));
                    return { 4, __vector };
                case 0xE:
                    return { 4, __vector };
                case 0xF:
                    return { 0, __vector };
                default:
                    raze_assert_unreachable();
            }
        }
        else {
            constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);
            alignas(16) _Type_ __source[__length], __result[__length];

            _mm_store_si128(reinterpret_cast<__m128i*>(__source), __as<__m128i>(__vector));

            _Type_* __result_pointer = __result;
            auto __start = __result_pointer;

            for (auto __index = 0; __index < __length; ++__index)
                if (!((__mask >> __index) & 1))
                    *__result_pointer++ = __source[__index];

            const auto __processed_size   = (__result_pointer - __start);
            const auto __processed_bytes  = __processed_size * sizeof(_Type_);

            std::memcpy(__result_pointer, __source + __processed_size, sizeof(_IntrinType_) - __processed_bytes);
            return { __processed_bytes, __as<_IntrinType_>(_mm_loadu_si128(reinterpret_cast<const __m128i*>(__result))) };
        }
	}
};

template <class _Type_> struct _Compress<arch::ISA::SSE3, 128, _Type_> : _Compress<arch::ISA::SSE2, 128, _Type_> {};

template <class _Type_> 
struct _Compress<arch::ISA::SSSE3, 128, _Type_>:
    _Compress<arch::ISA::SSE3, 128, _Type_>
{
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
    { 
        return (*this)(__vector, _To_mask<arch::ISA::SSE2, 128, _Type_>()(__mask));
    }

    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        if constexpr (sizeof(_Type_) == 1) {
            const auto __mask_segment_lower = __mask & 0xFF;
            const auto __mask_segment_higher = (__mask >> 8) & 0xFF;

            constexpr auto __vector_element_capacity = sizeof(_IntrinType_) / sizeof(_Type_);
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__vector_element_capacity];

            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __processed_byte_count_lower_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_lower];
            const auto __processed_byte_count_higher_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_higher];

            const auto __total_processed_byte_count_combined = __processed_byte_count_lower_segment + __processed_byte_count_higher_segment;
            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x10 / sizeof(_Type_)>(
                1u << (sizeof(_IntrinType_) - __total_processed_byte_count_combined)) - 1) << __total_processed_byte_count_combined;

            const auto __shuffle_control_mask_lower_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_lower]));
            const auto __shuffle_control_mask_higher_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_higher]));

            const auto __vector_upper_half_prepared_for_shuffling = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__as<__m128i>(__vector), 8)),
                __as<__m128>(__vector)));

            const auto __packed_data_lower_segment = _mm_shuffle_epi8(__as<__m128i>(__vector), __shuffle_control_mask_lower_segment);
            const auto __packed_data_higher_segment = _mm_shuffle_epi8(__vector_upper_half_prepared_for_shuffling, __as<__m128i>(__shuffle_control_mask_higher_segment));

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_lower_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_lower_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_higher_segment));

            const auto __final_packed_vector = _mm_load_si128(reinterpret_cast<const __m128i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::SSSE3, 128, _Type_>()(__as<__m128i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
        else {
            const auto __shuffle_mask = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask]));
            const auto __processed_bytes = __tables_sse<sizeof(_Type_)>.__size[__mask];

            __vector = __as<_IntrinType_>(_mm_shuffle_epi8(__as<__m128i>(__vector), __shuffle_mask));
            return { __processed_bytes, __vector };
        }
    }
};

template <class _Type_>
struct _Compress<arch::ISA::SSE41, 128, _Type_> :
	_Compress<arch::ISA::SSSE3, 128, _Type_>
{
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
        requires(__is_intrin_type_v<_MaskType_>)
	{
        return (*this)(__vector, _To_mask<arch::ISA::SSE2, 128, _Type_>()(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept\
            requires(std::is_integral_v<_MaskType_>)
	{
        if constexpr (sizeof(_Type_) == 1) {
            const auto __mask_segment_lower = __mask & 0xFF;
            const auto __mask_segment_higher = (__mask >> 8) & 0xFF;

            constexpr auto __vector_element_capacity = sizeof(_IntrinType_) / sizeof(_Type_);
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__vector_element_capacity];

            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __processed_byte_count_lower_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_lower];
            const auto __processed_byte_count_higher_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_higher];

            const auto __total_processed_byte_count_combined = __processed_byte_count_lower_segment + __processed_byte_count_higher_segment;
            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x10 / sizeof(_Type_)>(
                1u << (sizeof(_IntrinType_) - __total_processed_byte_count_combined)) - 1) << __total_processed_byte_count_combined;

            const auto __shuffle_control_mask_lower_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_lower]));
            const auto __shuffle_control_mask_higher_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_higher]));

            const auto __vector_upper_half_prepared_for_shuffling = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__as<__m128i>(__vector), 8)),
                __as<__m128>(__vector)));

            const auto __packed_data_lower_segment = _mm_shuffle_epi8(__as<__m128i>(__vector), __shuffle_control_mask_lower_segment);
            const auto __packed_data_higher_segment = _mm_shuffle_epi8(__vector_upper_half_prepared_for_shuffling, __as<__m128i>(__shuffle_control_mask_higher_segment));

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_lower_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_lower_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_higher_segment));

            const auto __final_packed_vector = _mm_load_si128(reinterpret_cast<const __m128i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::SSE41, 128, _Type_>()(__as<__m128i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
        else {
            const auto __shuffle_mask = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask]));
            const auto __processed_bytes = __tables_sse<sizeof(_Type_)>.__size[__mask];

            __vector = __as<_IntrinType_>(_mm_shuffle_epi8(__as<__m128i>(__vector), __shuffle_mask));
            return { __processed_bytes, __vector };
        }
	}
};

template <class _Type_>
struct _Compress<arch::ISA::AVX, 256, _Type_> {
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
	{
        return (*this)(__vector, _To_mask<arch::ISA::AVX, 256, _Type_>()(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
	{
        constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) == 8) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __lower_lane_vector = __as<__m128i>(__vector);
            const auto __higher_lane_vector = _mm256_extracti128_si256(__as<__m256i>(__vector), 1);

            const auto __mask_segment_lower = __mask & 0x03;
            const auto __mask_segment_higher = (__mask >> 2) & 0x03;

            const auto __processed_byte_count_lower_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_lower];
            const auto __processed_byte_count_higher_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_higher];

            const auto __total_processed_byte_count_combined = __processed_byte_count_lower_segment + __processed_byte_count_higher_segment;
            const auto __total_processed_element_count_combined = __total_processed_byte_count_combined / sizeof(_Type_);

            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x20 / sizeof(_Type_)>(
                1u << (__length - __total_processed_element_count_combined)) - 1) << __total_processed_element_count_combined;

            const auto __shuffle_control_mask_lower_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_lower]));
            const auto __shuffle_control_mask_higher_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_higher]));

            const auto __packed_data_lower_segment = _mm_shuffle_epi8(__lower_lane_vector, __shuffle_control_mask_lower_segment);
            const auto __packed_data_higher_segment = _mm_shuffle_epi8(__higher_lane_vector, __shuffle_control_mask_higher_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_lower_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_lower_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_higher_segment));

            const auto __final_packed_vector = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX, 256, _Type_>()(__as<__m256i>(__vector),
                __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
        else if constexpr (sizeof(_Type_) == 4) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __lower_lane_vector = __as<__m128i>(__vector);
            const auto __higher_lane_vector = _mm256_extracti128_si256(__as<__m256i>(__vector), 1);

            const auto __mask_segment_lower = __mask & 0xF;
            const auto __mask_segment_higher = (__mask >> 4) & 0xF;

            const auto __processed_byte_count_lower_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_lower];
            const auto __processed_byte_count_higher_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_higher];

            const auto __total_processed_byte_count_combined = __processed_byte_count_lower_segment + __processed_byte_count_higher_segment;
            const auto __total_processed_element_count_combined = __total_processed_byte_count_combined / sizeof(_Type_);

            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x20 / sizeof(_Type_)>(
                1u << (__length - __total_processed_element_count_combined)) - 1) << __total_processed_element_count_combined;

            const auto __shuffle_control_mask_lower_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_lower]));
            const auto __shuffle_control_mask_higher_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_higher]));

            const auto __packed_data_lower_segment = _mm_shuffle_epi8(__lower_lane_vector, __shuffle_control_mask_lower_segment);
            const auto __packed_data_higher_segment = _mm_shuffle_epi8(__higher_lane_vector, __shuffle_control_mask_higher_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_lower_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_lower_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_higher_segment));

            const auto __final_packed_vector = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX, 256, _Type_>()(__as<__m256i>(__vector),
                __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
        else if constexpr (sizeof(_Type_) == 2) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __lower_lane_vector = __as<__m128i>(__vector);
            const auto __higher_lane_vector = _mm256_extracti128_si256(__as<__m256i>(__vector), 1);

            const auto __mask_segment_lower = __mask & 0xFF;
            const auto __mask_segment_higher = (__mask >> 8) & 0xFF;

            const auto __processed_byte_count_lower_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_lower];
            const auto __processed_byte_count_higher_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_higher];

            const auto __total_processed_byte_count_combined = __processed_byte_count_lower_segment + __processed_byte_count_higher_segment;
            const auto __total_processed_element_count_combined = __total_processed_byte_count_combined / sizeof(_Type_);

            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x20 / sizeof(_Type_)>(
                1u << (__length - __total_processed_element_count_combined)) - 1) << __total_processed_element_count_combined;

            const auto __shuffle_control_mask_lower_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_lower]));
            const auto __shuffle_control_mask_higher_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_higher]));

            const auto __packed_data_lower_segment = _mm_shuffle_epi8(__lower_lane_vector, __shuffle_control_mask_lower_segment);
            const auto __packed_data_higher_segment = _mm_shuffle_epi8(__higher_lane_vector, __shuffle_control_mask_higher_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_lower_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_lower_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_higher_segment));

            const auto __final_packed_vector = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX2, 256, _Type_>()(__as<__m256i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
        else if constexpr (sizeof(_Type_) == 1) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __lower_lane_vector = __as<__m128i>(__vector);
            const auto __higher_lane_vector = _mm256_extracti128_si256(__as<__m256i>(__vector), 1);

            const auto __lower_lane_upper_half_vector = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__lower_lane_vector, 8)),
                __as<__m128>(__lower_lane_vector)));

            const auto __higher_lane_upper_half_vector = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__higher_lane_vector, 8)),
                __as<__m128>(__higher_lane_vector)));

            const auto __mask_segment_first = __mask & 0xFF;
            const auto __mask_segment_second = (__mask >> 8) & 0xFF;
            const auto __mask_segment_third = (__mask >> 16) & 0xFF;
            const auto __mask_segment_fourth = (__mask >> 24) & 0xFF;

            const auto __processed_byte_count_first_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_first];
            const auto __processed_byte_count_second_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_second];
            const auto __processed_byte_count_third_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_third];
            const auto __processed_byte_count_fourth_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_fourth];

            const auto __total_processed_byte_count_lower_lane = __processed_byte_count_first_segment + __processed_byte_count_second_segment;
            const auto __total_processed_byte_count_higher_lane = __processed_byte_count_third_segment + __processed_byte_count_fourth_segment;

            const auto __total_processed_byte_count_combined = __total_processed_byte_count_lower_lane + __total_processed_byte_count_higher_lane;
            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x20 / sizeof(_Type_)>(
                1u << (sizeof(_IntrinType_) - __total_processed_byte_count_combined)) - 1) << __total_processed_byte_count_combined;

            const auto __shuffle_control_mask_first_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_first]));
            const auto __shuffle_control_mask_second_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_second]));
            const auto __shuffle_control_mask_third_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_third]));
            const auto __shuffle_control_mask_fourth_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_fourth]));

            const auto __packed_data_first_segment = _mm_shuffle_epi8(__lower_lane_vector, __shuffle_control_mask_first_segment);
            const auto __packed_data_second_segment = _mm_shuffle_epi8(__lower_lane_upper_half_vector, __shuffle_control_mask_second_segment);
            const auto __packed_data_third_segment = _mm_shuffle_epi8(__higher_lane_vector, __shuffle_control_mask_third_segment);
            const auto __packed_data_fourth_segment = _mm_shuffle_epi8(__higher_lane_upper_half_vector, __shuffle_control_mask_fourth_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_first_segment));
            __destination_write_pointer += __processed_byte_count_first_segment;

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_second_segment));
            __destination_write_pointer += __processed_byte_count_second_segment;

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_third_segment));
            __destination_write_pointer += __processed_byte_count_third_segment;

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_fourth_segment));

            const auto __final_packed_vector = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX2, 256, _Type_>()(__as<__m256i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
	}
};

template <class _Type_> struct _Compress<arch::ISA::FMA3, 256, _Type_> : _Compress<arch::ISA::AVX, 256, _Type_> {};

template <class _Type_>
struct _Compress<arch::ISA::AVX2, 256, _Type_> {
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
	{
        return (*this)(__vector, _To_mask<arch::ISA::AVX2, 256, _Type_>()(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
	{
        constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) >= 4) {
            const auto __processed_bytes = __tables_avx<sizeof(_Type_)>.__size[__mask];
            const auto __shuffle = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_avx<sizeof(_Type_)>.__shuffle[__mask]));

            const auto __converted_shuffle = _mm256_cvtepu8_epi32(__shuffle);
            __vector = __as<_IntrinType_>(_mm256_permutevar8x32_epi32(__as<__m256i>(__vector), __converted_shuffle));

            return { __processed_bytes, __vector };
        }
        else if constexpr (sizeof(_Type_) == 2) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __lower_lane_vector = __as<__m128i>(__vector);
            const auto __higher_lane_vector = _mm256_extracti128_si256(__as<__m256i>(__vector), 1);

            const auto __mask_segment_lower = __mask & 0xFF;
            const auto __mask_segment_higher = (__mask >> 8) & 0xFF;

            const auto __processed_byte_count_lower_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_lower];
            const auto __processed_byte_count_higher_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_higher];

            const auto __total_processed_byte_count_combined = __processed_byte_count_lower_segment + __processed_byte_count_higher_segment;
            const auto __total_processed_element_count_combined = __total_processed_byte_count_combined / sizeof(_Type_);

            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x20 / sizeof(_Type_)>(
                1u << (__length - __total_processed_element_count_combined)) - 1) << __total_processed_element_count_combined;

            const auto __shuffle_control_mask_lower_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_lower]));
            const auto __shuffle_control_mask_higher_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_higher]));

            const auto __packed_data_lower_segment = _mm_shuffle_epi8(__lower_lane_vector, __shuffle_control_mask_lower_segment);
            const auto __packed_data_higher_segment = _mm_shuffle_epi8(__higher_lane_vector, __shuffle_control_mask_higher_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_lower_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_lower_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_higher_segment));

            const auto __final_packed_vector = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX2, 256, _Type_>()(__as<__m256i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
        else if constexpr (sizeof(_Type_) == 1) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __lower_lane_vector = __as<__m128i>(__vector);
            const auto __higher_lane_vector = _mm256_extracti128_si256(__as<__m256i>(__vector), 1);

            const auto __lower_lane_upper_half_vector = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__lower_lane_vector, 8)),
                __as<__m128>(__lower_lane_vector)));

            const auto __higher_lane_upper_half_vector = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__higher_lane_vector, 8)),
                __as<__m128>(__higher_lane_vector)));

            const auto __mask_segment_first = __mask & 0xFF;
            const auto __mask_segment_second = (__mask >> 8) & 0xFF;
            const auto __mask_segment_third = (__mask >> 16) & 0xFF;
            const auto __mask_segment_fourth = (__mask >> 24) & 0xFF;

            const auto __processed_byte_count_first_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_first];
            const auto __processed_byte_count_second_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_second];
            const auto __processed_byte_count_third_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_third];
            const auto __processed_byte_count_fourth_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_fourth];

            const auto __total_processed_byte_count_lower_lane = __processed_byte_count_first_segment + __processed_byte_count_second_segment;
            const auto __total_processed_byte_count_higher_lane = __processed_byte_count_third_segment + __processed_byte_count_fourth_segment;

            const auto __total_processed_byte_count_combined = __total_processed_byte_count_lower_lane + __total_processed_byte_count_higher_lane;
            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x20 / sizeof(_Type_)>(
                1u << (sizeof(_IntrinType_) - __total_processed_byte_count_combined)) - 1) << __total_processed_byte_count_combined;

            const auto __shuffle_control_mask_first_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_first]));
            const auto __shuffle_control_mask_second_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_second]));
            const auto __shuffle_control_mask_third_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_third]));
            const auto __shuffle_control_mask_fourth_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_fourth]));

            const auto __packed_data_first_segment = _mm_shuffle_epi8(__lower_lane_vector, __shuffle_control_mask_first_segment);
            const auto __packed_data_second_segment = _mm_shuffle_epi8(__lower_lane_upper_half_vector, __shuffle_control_mask_second_segment);
            const auto __packed_data_third_segment = _mm_shuffle_epi8(__higher_lane_vector, __shuffle_control_mask_third_segment);
            const auto __packed_data_fourth_segment = _mm_shuffle_epi8(__higher_lane_upper_half_vector, __shuffle_control_mask_fourth_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_first_segment));
            __destination_write_pointer += __processed_byte_count_first_segment;

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_second_segment));
            __destination_write_pointer += __processed_byte_count_second_segment;

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_third_segment));
            __destination_write_pointer += __processed_byte_count_third_segment;

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_fourth_segment));

            const auto __final_packed_vector = _mm256_load_si256(reinterpret_cast<const __m256i*>(__temporary_stack_buffer));
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX2, 256, _Type_>()(__as<__m256i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
	}
};

template <class _Type_>
struct _Compress<arch::ISA::AVX512F, 512, _Type_> {
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
        requires(__is_intrin_type_v<_MaskType_>)
	{
        return (*this)(__vector, _To_mask<arch::ISA::AVX512F, 512, _Type_>()(__mask));
	}

	template <
		class _IntrinType_,
		class _MaskType_>
	raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
		_IntrinType_	__vector,
		_MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
	{
        constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) == 8) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = (math::population_count(__to_gpr<arch::ISA::AVX512F>(__not)) << 3);

            return { __processed_bytes, __as<_IntrinType_>(_mm512_mask_compress_epi64(
                __as<__m512i>(__vector), __not, __as<__m512i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 4) {
            const auto __not = _knot_mask16(__mask);
            const auto __processed_bytes = (math::population_count(__to_gpr<arch::ISA::AVX512F>(__not)) << 2);

            return { __processed_bytes, __as<_IntrinType_>(_mm512_mask_compress_epi32(
                __as<__m512i>(__vector), __not, __as<__m512i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 2) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __xmm_lane_vector1 = __as<__m128i>(__vector);
            const auto __xmm_lane_vector2 = _mm256_extracti128_si256(__as<__m256i>(__vector), 1);
            const auto __xmm_lane_vector3 = __as<__m128i>(_mm512_extractf32x4_ps(__as<__m512>(__vector), 2));
            const auto __xmm_lane_vector4 = __as<__m128i>(_mm512_extractf32x4_ps(__as<__m512>(__vector), 3));

            const auto __mask_segment_first = __mask & 0xFF;
            const auto __mask_segment_second = (__mask >> 8) & 0xFF;
            const auto __mask_segment_third = (__mask >> 16) & 0xFF;
            const auto __mask_segment_fourth = (__mask >> 24) & 0xFF;

            const auto __processed_byte_count_first_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_first];
            const auto __processed_byte_count_second_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_second];
            const auto __processed_byte_count_third_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_third];
            const auto __processed_byte_count_fourth_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_fourth];

            const auto __total_processed_byte_count_combined = __processed_byte_count_first_segment + __processed_byte_count_second_segment
                + __processed_byte_count_third_segment + __processed_byte_count_fourth_segment;
            const auto __total_processed_element_count_combined = __total_processed_byte_count_combined / sizeof(_Type_);

            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x40 / sizeof(_Type_)>(
                1u << (__length - __total_processed_element_count_combined)) - 1) << __total_processed_element_count_combined;

            const auto __shuffle_control_mask_first_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_first]));
            const auto __shuffle_control_mask_second_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_second]));
            const auto __shuffle_control_mask_third_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_third]));
            const auto __shuffle_control_mask_fourth_segment = _mm_load_si128(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_fourth]));

            const auto __packed_data_first_segment = _mm_shuffle_epi8(__xmm_lane_vector1, __shuffle_control_mask_first_segment);
            const auto __packed_data_second_segment = _mm_shuffle_epi8(__xmm_lane_vector2, __shuffle_control_mask_second_segment);
            const auto __packed_data_third_segment = _mm_shuffle_epi8(__xmm_lane_vector3, __shuffle_control_mask_third_segment);
            const auto __packed_data_fourth_segment = _mm_shuffle_epi8(__xmm_lane_vector4, __shuffle_control_mask_fourth_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_first_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_first_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_second_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_second_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_third_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_third_segment);

            _mm_store_si128(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_fourth_segment));

            const auto __final_packed_vector = _mm512_load_si512(__temporary_stack_buffer);
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX512F, 512, _Type_>()(
                __as<__m512i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
        else if constexpr (sizeof(_Type_) == 1) {
            alignas(sizeof(_IntrinType_)) _Type_ __temporary_stack_buffer[__length];
            _Type_* __destination_write_pointer = __temporary_stack_buffer;

            const auto __ymm_lower_lane_vector = __as<__m256i>(__vector);;
            const auto __ymm_higher_lane_vector = _mm512_extractf64x4_pd(__as<__m512d>(__vector), 1);

            const auto __xmm_lane_vector1 = __as<__m128i>(__ymm_lower_lane_vector);
            const auto __xmm_lane_vector2 = __as<__m128i>(_mm256_extractf128_pd(__as<__m256d>(__ymm_lower_lane_vector), 1));

            const auto __xmm_lane_vector3 = __as<__m128i>(__ymm_higher_lane_vector);
            const auto __xmm_lane_vector4 = __as<__m128i>(_mm256_extractf128_pd(__as<__m256d>(__ymm_higher_lane_vector), 1));

            const auto __xmm_lane_upper_half_vector1 = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__xmm_lane_vector1, 8)),
                __as<__m128>(__xmm_lane_vector1)));

            const auto __xmm_lane_upper_half_vector2 = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__xmm_lane_vector2, 8)),
                __as<__m128>(__xmm_lane_vector2)));

            const auto __xmm_lane_upper_half_vector3 = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__xmm_lane_vector3, 8)),
                __as<__m128>(__xmm_lane_vector3)));

            const auto __xmm_lane_upper_half_vector4 = __as<__m128i>(_mm_movehl_ps(
                __as<__m128>(_mm_slli_si128(__xmm_lane_vector4, 8)),
                __as<__m128>(__xmm_lane_vector4)));

            const auto __mask_segment_first = __mask & 0xFF;
            const auto __mask_segment_second = (__mask >> 8) & 0xFF;
            const auto __mask_segment_third = (__mask >> 16) & 0xFF;
            const auto __mask_segment_fourth = (__mask >> 24) & 0xFF;
            const auto __mask_segment_fifth = (__mask >> 32) & 0xFF;
            const auto __mask_segment_sixth = (__mask >> 40) & 0xFF;
            const auto __mask_segment_seventh = (__mask >> 48) & 0xFF;
            const auto __mask_segment_eighth = (__mask >> 56) & 0xFF;

            const auto __processed_byte_count_first_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_first];
            const auto __processed_byte_count_second_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_second];
            const auto __processed_byte_count_third_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_third];
            const auto __processed_byte_count_fourth_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_fourth];
            const auto __processed_byte_count_fifth_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_fifth];
            const auto __processed_byte_count_sixth_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_sixth];
            const auto __processed_byte_count_seventh_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_seventh];
            const auto __processed_byte_count_eighth_segment = __tables_sse<sizeof(_Type_)>.__size[__mask_segment_eighth];

            const auto __total_processed_byte_count_xmm_lane1 = __processed_byte_count_first_segment + __processed_byte_count_second_segment;
            const auto __total_processed_byte_count_xmm_lane2 = __processed_byte_count_third_segment + __processed_byte_count_fourth_segment;
            const auto __total_processed_byte_count_xmm_lane3 = __processed_byte_count_fifth_segment + __processed_byte_count_sixth_segment;
            const auto __total_processed_byte_count_xmm_lane4 = __processed_byte_count_seventh_segment + __processed_byte_count_eighth_segment;

            const auto __total_processed_byte_count_combined = __total_processed_byte_count_xmm_lane1 + __total_processed_byte_count_xmm_lane2
                + __total_processed_byte_count_xmm_lane3 + __total_processed_byte_count_xmm_lane4;

            const auto __unprocessed_tail_blending_mask = (__mmask_for_elements_t<0x40 / sizeof(_Type_)>(
                uint64(1) << (__length - __total_processed_byte_count_combined)) - 1) << __total_processed_byte_count_combined;

            const auto __shuffle_control_mask_first_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_first]));
            const auto __shuffle_control_mask_second_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_second]));
            const auto __shuffle_control_mask_third_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_third]));
            const auto __shuffle_control_mask_fourth_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_fourth]));
            const auto __shuffle_control_mask_fifth_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_fifth]));
            const auto __shuffle_control_mask_sixth_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_sixth]));
            const auto __shuffle_control_mask_seventh_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_seventh]));
            const auto __shuffle_control_mask_eighth_segment = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(__tables_sse<sizeof(_Type_)>.__shuffle[__mask_segment_eighth]));

            const auto __packed_data_first_segment = _mm_shuffle_epi8(__xmm_lane_vector1, __shuffle_control_mask_first_segment);
            const auto __packed_data_second_segment = _mm_shuffle_epi8(__xmm_lane_upper_half_vector1, __shuffle_control_mask_second_segment);
            const auto __packed_data_third_segment = _mm_shuffle_epi8(__xmm_lane_vector2, __shuffle_control_mask_third_segment);
            const auto __packed_data_fourth_segment = _mm_shuffle_epi8(__xmm_lane_upper_half_vector2, __shuffle_control_mask_fourth_segment);
            const auto __packed_data_fifth_segment = _mm_shuffle_epi8(__xmm_lane_vector3, __shuffle_control_mask_fifth_segment);
            const auto __packed_data_sixth_segment = _mm_shuffle_epi8(__xmm_lane_upper_half_vector3, __shuffle_control_mask_sixth_segment);
            const auto __packed_data_seventh_segment = _mm_shuffle_epi8(__xmm_lane_vector4, __shuffle_control_mask_seventh_segment);
            const auto __packed_data_eighth_segment = _mm_shuffle_epi8(__xmm_lane_upper_half_vector4, __shuffle_control_mask_eighth_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_first_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_first_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_second_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_second_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_third_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_third_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_fourth_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_fourth_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_fifth_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_fifth_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_sixth_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_sixth_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_seventh_segment));
            algorithm::__advance_bytes(__destination_write_pointer, __processed_byte_count_seventh_segment);

            _mm_storel_epi64(reinterpret_cast<__m128i*>(__destination_write_pointer), __as<__m128i>(__packed_data_eighth_segment));

            const auto __final_packed_vector = _mm512_load_si512(__temporary_stack_buffer);
            const auto __final_blended_result_vector = _Select<arch::ISA::AVX512F, 512, _Type_>()(__as<__m512i>(__vector), __final_packed_vector, __unprocessed_tail_blending_mask);

            return { __total_processed_byte_count_combined, __as<_IntrinType_>(__final_blended_result_vector) };
        }
	}
};

template <class _Type_> 
struct _Compress<arch::ISA::AVX512VLF, 256, _Type_>:
    _Compress<arch::ISA::AVX2, 256, _Type_> 
{
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
    {
        return (*this)(__vector, _To_mask<arch::ISA::AVX512VLF, 256, _Type_>()(__mask));
    }

    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_	__vector,
        _MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) == 8) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = (math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VLF>(__not)) << 3);

            return { __processed_bytes, __as<_IntrinType_>(_mm256_mask_compress_epi64(
                __as<__m256i>(__vector), __not, __as<__m256i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 4) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = (math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VLF>(__not)) << 2);

            return { __processed_bytes, __as<_IntrinType_>(_mm256_mask_compress_epi32(
                __as<__m256i>(__vector), __not, __as<__m256i>(__vector))) };
        }
        else {
            return _Compress<arch::ISA::AVX2, 256, _Type_>()(__vector, __mask);
        }
    }
};

template <class _Type_>
struct _Compress<arch::ISA::AVX512VLF, 128, _Type_> :
    _Compress<arch::ISA::AVX2, 128, _Type_>
{
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
    {
        return (*this)(__vector, _To_mask<arch::ISA::AVX512VLF, 128, _Type_>()(__mask));
    }

    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_	__vector,
        _MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) == 8) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = (math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VLF>(__not)) << 3);

            return { __processed_bytes, __as<_IntrinType_>(_mm_mask_compress_epi64(
                __as<__m128i>(__vector), __not, __as<__m128i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 4) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = (math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VLF>(__not)) << 2);

            return { __processed_bytes, __as<_IntrinType_>(_mm_mask_compress_epi32(
                __as<__m128i>(__vector), __not, __as<__m128i>(__vector))) };
        }
        else {
            return _Compress<arch::ISA::SSE42, 128, _Type_>()(__vector, __mask);
        }
    }
};

template <class _Type_> 
struct _Compress<arch::ISA::AVX512VBMI2, 512, _Type_>:
    _Compress<arch::ISA::AVX512VBMI, 512, _Type_> 
{
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
    {
        return (*this)(__vector, _To_mask<arch::ISA::AVX512VBMI2, 512, _Type_>()(__mask));
    }

    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_	__vector,
        _MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) == 8) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__to_gpr<arch::ISA::AVX512VBMI2>(__mask)));
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2>(__not)) << 3;

            return { __processed_bytes, __as<_IntrinType_>(_mm512_mask_compress_epi64(
                __as<__m512i>(__vector), __not, __as<__m512i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 4) {
            const auto __not = _knot_mask16(__mask);
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2>(__not)) << 2;

            return { __processed_bytes, __as<_IntrinType_>(_mm512_mask_compress_epi32(
                __as<__m512i>(__vector), __not, __as<__m512i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 2) {
            const auto __not = _knot_mask32(__mask);
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2>(__not)) << 1;

            return { __processed_bytes, __as<_IntrinType_>(_mm512_mask_compress_epi16(
                __as<__m512i>(__vector), __not, __as<__m512i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 1) {
            const auto __not = _knot_mask64(__mask);
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2>(__not));

            return { __processed_bytes, __as<_IntrinType_>(_mm512_mask_compress_epi8(
                __as<__m512i>(__vector), __not, __as<__m512i>(__vector))) };
        }
    }
};

template <class _Type_> 
struct _Compress<arch::ISA::AVX512VBMI2VL, 256, _Type_>:
    _Compress<arch::ISA::AVX512VBMIVL, 256, _Type_> 
{
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
    {
        return (*this)(__vector, _To_mask<arch::ISA::AVX512VBMI2VL, 256, _Type_>()(__mask));
    }

    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_	__vector,
        _MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        constexpr auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) == 8) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not)) << 3;

            return { __processed_bytes, __as<_IntrinType_>(_mm256_mask_compress_epi64(
                __as<__m256i>(__vector), __not, __as<__m256i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 4) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not)) << 2;

            return { __processed_bytes, __as<_IntrinType_>(_mm256_mask_compress_epi32(
                __as<__m256i>(__vector), __not, __as<__m256i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 2) {
            const auto __not = _knot_mask16(__mask);
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not)) << 1;

            return { __processed_bytes, __as<_IntrinType_>(_mm256_mask_compress_epi16(
                __as<__m256i>(__vector), __not, __as<__m256i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 1) {
            const auto __not = _knot_mask32(__mask);
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not));

            return { __processed_bytes, __as<_IntrinType_>(_mm256_mask_compress_epi8(
                __as<__m256i>(__vector), __not, __as<__m256i>(__vector))) };
        }
    }
};

template <class _Type_> 
struct _Compress<arch::ISA::AVX512VBMI2VL, 128, _Type_>:
    _Compress<arch::ISA::AVX512VBMIVL, 128, _Type_>
{
    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_    __vector,
        _MaskType_      __mask) const noexcept
            requires(__is_intrin_type_v<_MaskType_>)
    {
        return (*this)(__vector, _To_mask<arch::ISA::AVX512VBMI2VL, 128, _Type_>()(__mask));
    }

    template <
        class _IntrinType_,
        class _MaskType_>
    raze_nodiscard raze_always_inline std::pair<int32, _IntrinType_> operator()(
        _IntrinType_	__vector,
        _MaskType_		__mask) const noexcept
            requires(std::is_integral_v<_MaskType_>)
    {
        const auto __length = sizeof(_IntrinType_) / sizeof(_Type_);

        if constexpr (sizeof(_Type_) == 8) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not)) << 3;

            return { __processed_bytes, __as<_IntrinType_>(_mm_mask_compress_epi64(
                __as<__m128i>(__vector), __not, __as<__m128i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 4) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not)) << 2;

            return { __processed_bytes, __as<_IntrinType_>(_mm_mask_compress_epi32(
                __as<__m128i>(__vector), __not, __as<__m128i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 2) {
            const auto __not = static_cast<__mmask8>(_knot_mask16(__mask));
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not)) << 2;

            return { __processed_bytes, __as<_IntrinType_>(_mm_mask_compress_epi16(
                __as<__m128i>(__vector), __not, __as<__m128i>(__vector))) };
        }
        else if constexpr (sizeof(_Type_) == 1) {
            const auto __not = _knot_mask16(__mask);
            const auto __processed_bytes = math::__native_popcnt_n_bits<__length>(__to_gpr<arch::ISA::AVX512VBMI2VL>(__not));

            return { __processed_bytes, __as<_IntrinType_>(_mm_mask_compress_epi8(
                __as<__m128i>(__vector), __not, __as<__m128i>(__vector))) };
        }
    }
};


template <class _Type_> struct _Compress<arch::ISA::SSE42, 128, _Type_> : _Compress<arch::ISA::SSE41, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX, 128, _Type_> : _Compress<arch::ISA::SSE42, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::FMA3, 128, _Type_> : _Compress<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX2, 128, _Type_> : _Compress<arch::ISA::AVX, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX2FMA3, 128, _Type_> : _Compress<arch::ISA::AVX2, 128, _Type_> {};

template <class _Type_> struct _Compress<arch::ISA::AVX512BW, 512, _Type_> : _Compress<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512DQ, 512, _Type_> : _Compress<arch::ISA::AVX512F, 512, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512BWDQ, 512, _Type_> : _Compress<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMI, 512, _Type_> : _Compress<arch::ISA::AVX512BW, 512, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMIDQ, 512, _Type_> : _Compress<arch::ISA::AVX512BWDQ, 512, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMI2DQ, 512, _Type_> : _Compress<arch::ISA::AVX512VBMIDQ, 512, _Type_> {};

template <class _Type_> struct _Compress<arch::ISA::AVX2FMA3, 256, _Type_> : _Compress<arch::ISA::AVX2, 256, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VLBW, 256, _Type_> : _Compress<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VLDQ, 256, _Type_> : _Compress<arch::ISA::AVX512VLF, 256, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VLBWDQ, 256, _Type_> : _Compress<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMIVL, 256, _Type_> : _Compress<arch::ISA::AVX512VLBW, 256, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> : _Compress<arch::ISA::AVX512VLBWDQ, 256, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMI2VLDQ, 256, _Type_> : _Compress<arch::ISA::AVX512VBMIVLDQ, 256, _Type_> {};

template <class _Type_> struct _Compress<arch::ISA::AVX512VLBW, 128, _Type_> : _Compress<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VLDQ, 128, _Type_> : _Compress<arch::ISA::AVX512VLF, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VLBWDQ, 128, _Type_> : _Compress<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMIVL, 128, _Type_> : _Compress<arch::ISA::AVX512VLBW, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> : _Compress<arch::ISA::AVX512VLBWDQ, 128, _Type_> {};
template <class _Type_> struct _Compress<arch::ISA::AVX512VBMI2VLDQ, 128, _Type_> : _Compress<arch::ISA::AVX512VBMIVLDQ, 128, _Type_> {};

__RAZE_VX_NAMESPACE_END
