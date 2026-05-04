#pragma once 

#include <src/raze/vx/storage/BestChunk.h>

__RAZE_VX_NAMESPACE_BEGIN

template <u64 _N_>
using __mmask_for_elements_helper = std::conditional_t<_N_ <= 8, __mmask8,
    std::conditional_t<_N_ <= 16, __mmask16,
        std::conditional_t<_N_ <= 32, __mmask32,
            std::conditional_t<_N_ <= 64, __mmask64, void>>>>;

template <u64 _N_>
using __mmask_for_elements_t = __mmask_for_elements_helper<_N_>;

template <class _Type_, class _Abi_, u64 _Elements_, raw_mask_type _Mask_>
struct _Mask_wrapper {
    using mask_type = _Mask_;
    using abi_type = _Abi_;
    using value_type = _Type_;

    static constexpr auto size = _Elements_;

    _Mask_wrapper(mask_type __mask) noexcept : _data(__mask) {}

    _Mask_wrapper() noexcept = default;
    _Mask_wrapper(const _Mask_wrapper&) noexcept = default;
    _Mask_wrapper(_Mask_wrapper&&) noexcept = default;

    ~_Mask_wrapper() = default;

    _Mask_wrapper& operator=(const _Mask_wrapper&) noexcept = default;
    _Mask_wrapper& operator=(_Mask_wrapper&&) noexcept = default;

    raze_nodiscard raze_always_inline mask_type data() const noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline mask_type& data() noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline operator mask_type() const noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline operator mask_type&() noexcept {
        return _data;
    }
private:
    mask_type _data;
};

template <class _Type_, class _Abi_, i32 _Remaining_>
struct best_mask_chunk {
    static constexpr auto __use_kmask = __has_avx512f_support_v<_Abi_::isa> && (sizeof(_Type_) >= 4 || __has_avx512bw_support_v<_Abi_::isa>);
    static constexpr auto __max_isa_width = __has_avx512f_support_v<_Abi_::isa> ? 512 : __has_avx_support_v<_Abi_::isa> ? 256 : __has_sse2_support_v<_Abi_::isa> ? 128 : 0;

    static constexpr auto __bytes = _Remaining_ * sizeof(_Type_);
    static constexpr auto __data_width = (__bytes >= 64) ? 512 :  (__bytes >= 32) ? 256 : (__bytes >= 16) ? 128 : 0;
    
    static constexpr auto __width = (__data_width < __max_isa_width) ? __data_width : __max_isa_width;
    static constexpr auto __fits = (__width != 0);

    static constexpr auto __elems_in_vector = (__width == 0) ? _Remaining_ : (__width / 8) / sizeof(_Type_);

    static constexpr auto elements_count = __fits ? __elems_in_vector : 1;

    using type = std::conditional_t<__use_kmask && __fits, _Mask_wrapper<_Type_, _Abi_, elements_count, __mmask_for_elements_t<__elems_in_vector>>,
        std::conditional_t<__fits, _Mask_wrapper<_Type_, _Abi_, elements_count, typename typename best_chunk<
            typename IntegerForSizeof<_Type_>::Unsigned, _Abi_, _Remaining_>::type::vector_type>, _Mask_wrapper<_Type_, _Abi_, elements_count, bool>>>;
};

template <class _Type_, class _Abi_, i32 _Remaining_>
struct __build_mask_tuple {
    using __chunk_info = best_mask_chunk<_Type_, _Abi_, _Remaining_>;
    using __chunk = typename __chunk_info::type;

    static constexpr auto __elems = __chunk_info::elements_count;
    static constexpr auto __next = (_Remaining_ > __elems) ? _Remaining_ - __elems : 0;

    using type = _Simd_tuple_node<__chunk, typename __build_mask_tuple<_Type_, _Abi_, __next>::type>;
};

template <class _Type_, class _Abi_>
struct __build_mask_tuple<_Type_, _Abi_, 0> {
    using type = _Simd_tuple_nil;
};

template <class _Type_, class _Abi_>
using _Simd_mask_tuple_type = typename __build_mask_tuple<_Type_, _Abi_, _Abi_::size>::type;

__RAZE_VX_NAMESPACE_END
