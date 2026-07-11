#pragma once 

#include <src/raze/vx/hw/Reduce.h>
#include <src/raze/vx/hw/Compact.h>
#include <src/raze/vx/hw/Compare.h>
#include <src/raze/vx/hw/Construct.h>
#include <src/raze/vx/hw/Shuffle.h>
#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/vx/hw/Bitwise.h>
#include <src/raze/vx/hw/Mask.h>
#include <src/raze/vx/hw/Merge.h>
#include <src/raze/vx/hw/Memory.h>
#include <src/raze/vx/AsTernaryMask.h>
#include <src/raze/vx/hw/Shape.h>

__RAZE_VX_NAMESPACE_BEGIN

/* abs, fma, fms, fnma, fnms, sin, cos, tan, cotan defined in raze/math/Math.h */
constexpr inline auto add = __add;
constexpr inline auto sub = raze::options::functor<_Configurable_sub>;
constexpr inline auto mul = raze::options::functor<_Configurable_mul>;
constexpr inline auto div = raze::options::functor<_Configurable_div>;
constexpr inline auto vertical_max = __vmax;
constexpr inline auto vertical_min = __vmin;
constexpr inline auto neg = raze::options::functor<_Configurable_neg>;
constexpr inline auto bit_or = raze::options::functor<_Configurable_or>;
constexpr inline auto bit_xor = raze::options::functor<_Configurable_xor>;
constexpr inline auto bit_and = raze::options::functor<_Configurable_and>;
constexpr inline auto bit_andnot = raze::options::functor<_Configurable_andnot>;
constexpr inline auto bit_shl = raze::options::functor<_Configurable_shl>;
constexpr inline auto bit_shr = raze::options::functor<_Configurable_shr>;
constexpr inline auto bit_not = raze::options::functor<_Configurable_not>;
constexpr inline auto horizontal_sum = __hsum;
constexpr inline auto horizontal_min = raze::options::functor<_Configurable_hmin>;
constexpr inline auto horizontal_max = raze::options::functor<_Configurable_hmax>;
constexpr inline auto all_of = raze::options::functor<_Configurable_all_of>;
constexpr inline auto none_of = raze::options::functor<_Configurable_none_of>;
constexpr inline auto any_of = raze::options::functor<_Configurable_any_of>;
constexpr inline auto some_of = raze::options::functor<_Configurable_some_of>;
constexpr inline auto find_first_set = raze::options::functor<_Configurable_find_first_set>;
constexpr inline auto find_last_set = raze::options::functor<_Configurable_find_last_set>;
constexpr inline auto find_first_not_set = raze::options::functor<_Configurable_find_first_not_set>;
constexpr inline auto find_last_not_set = raze::options::functor<_Configurable_find_last_not_set>;
constexpr inline auto count_set = raze::options::functor<_Configurable_count_set>;
constexpr inline auto is_contiguous = raze::options::functor<_Configurable_is_contiguous>;
constexpr inline auto is_less = raze::options::functor<_Configurable_is_less>;
constexpr inline auto is_greater = raze::options::functor<_Configurable_is_greater>;
constexpr inline auto is_less_equal = raze::options::functor<_Configurable_is_less_equal>;
constexpr inline auto is_greater_equal = raze::options::functor<_Configurable_is_greater_equal>;
constexpr inline auto is_equal = raze::options::functor<_Configurable_is_equal>;
constexpr inline auto is_not_equal = raze::options::functor<_Configurable_is_not_equal>;
constexpr inline auto select = __select;
constexpr inline auto ternarylogic = raze::options::functor<_Configurable_ternarylogic>;
template <simd_type _Simd_> constexpr inline auto load = __load<_Simd_>;
constexpr inline auto store = __store;
//constexpr inline auto compress_store = raze::options::functor<_Configurable_compress_store>;
constexpr inline auto clear_first = __clear_first_impl;
constexpr inline auto clear_last = __clear_last_impl;
constexpr inline auto concat = __concat;
constexpr inline auto split = __split;
template <sizetype _Chunks_> constexpr inline auto split_by = __split_by<_Chunks_>;
constexpr inline auto reverse = raze::options::functor<_Configurable_reverse>;
template <sizetype _Index_> constexpr inline auto splat = __splat_impl<_Index_>;
constexpr inline auto slide_left = raze::options::functor<_Configurable_slide_left>;
constexpr inline auto slide_left_merge = raze::options::functor<_Configurable_slide_left_merge>;
constexpr inline auto slide_right = raze::options::functor<_Configurable_slide_right>;
constexpr inline auto slide_right_merge = raze::options::functor<_Configurable_slide_right_merge>;
constexpr inline auto rotate_left = raze::options::functor<_Configurable_rotate_left>;
constexpr inline auto rotate_right = raze::options::functor<_Configurable_rotate_right>;
constexpr inline auto shuffle = raze::options::functor<_Configurable_shuffle>;
constexpr inline auto clamp = raze::options::functor<_Configurable_clamp>;
constexpr inline auto sfence = raze::options::functor<_Configurable_sfence>;
constexpr inline auto fold = raze::options::functor<_Configurable_fold>;
constexpr inline auto vmin = vertical_min;
constexpr inline auto vmax = vertical_max;
constexpr inline auto hsum = horizontal_sum;
constexpr inline auto hmin = horizontal_min;
constexpr inline auto hmax = horizontal_max;


template <class _Type_>
struct stream_ptr {
    using value_type = _Type_;
    using pointer = _Type_*;
    using const_pointer = const _Type_*;
    using reference = _Type_&;
    using const_reference = const _Type_&;

    pointer _ptr = nullptr;

    stream_ptr() noexcept {}
    explicit stream_ptr(pointer __p) noexcept:  
        _ptr(__p) 
    {}

    stream_ptr(const std::remove_const_t<_Type_>* __p) noexcept:
        _ptr(const_cast<pointer>(__p)) 
    {}

    ~stream_ptr() noexcept {
        sfence();
    }

    raze_always_inline pointer get() const noexcept {
        return _ptr;
    }

    raze_always_inline stream_ptr operator+(std::ptrdiff_t __offset) const noexcept {
        return stream_ptr(_ptr + __offset);
    }

    raze_always_inline stream_ptr operator-(std::ptrdiff_t __offset) const noexcept {
        return stream_ptr(_ptr - __offset);
    }

    raze_always_inline stream_ptr& operator+=(std::ptrdiff_t __offset) noexcept {
        _ptr += __offset;
        return *this;
    }

    raze_always_inline stream_ptr& operator-=(std::ptrdiff_t __offset) noexcept {
        _ptr -= __offset;
        return *this;
    }

    raze_always_inline stream_ptr& operator++() noexcept {
        ++_ptr;
        return *this;
    }

    raze_always_inline stream_ptr operator++(int) noexcept {
        stream_ptr __tmp = *this;
        ++_ptr;
        return __tmp;
    }

    raze_always_inline bool operator==(const stream_ptr& __other) const noexcept {
        return _ptr == __other._ptr;
    }

    raze_always_inline bool operator!=(const stream_ptr& __other) const noexcept {
        return _ptr != __other._ptr;
    }

    raze_always_inline bool operator<(const stream_ptr& __other) const noexcept {
        return _ptr < __other._ptr;
    }

    raze_always_inline void advance_bytes(sizetype __bytes) noexcept {
        algorithm::__advance_bytes(_ptr, __bytes);
    }
};

template <class _Type_>
raze_always_inline constexpr stream_ptr<std::remove_cv_t<_Type_>> stream(_Type_* __ptr) noexcept {
    return stream_ptr<std::remove_cv_t<_Type_>>(__ptr);
}

template <ternary_mask_expression_type _Expression_>
raze_always_inline constexpr auto as_ternary_mask() noexcept {
	return __as_ternary_mask<_Expression_>();
}

__RAZE_VX_NAMESPACE_END
