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

/* abs, fma, fms, fnma, fnms, sin, cos defined in raze/math/Math.h */
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
constexpr inline auto find_first_set = raze::options::functor<_Configurable_find_first_set>;
constexpr inline auto find_next_set = raze::options::functor<_Configurable_find_next_set>;
constexpr inline auto find_last_set = raze::options::functor<_Configurable_find_last_set>;
constexpr inline auto find_first_not_set = raze::options::functor<_Configurable_find_first_not_set>;
constexpr inline auto find_last_not_set = raze::options::functor<_Configurable_find_last_not_set>;
constexpr inline auto count_set = raze::options::functor<_Configurable_count_set>;
constexpr inline auto is_contiguous = raze::options::functor<_Configurable_is_contiguous>;
constexpr inline auto first_n = raze::options::functor<_Configurable_first_n>;
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
constexpr inline auto compress_store = raze::options::functor<_Configurable_compress_store>;
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
constexpr inline auto swap_adjacent = __swap_adjacent;
constexpr inline auto shuffle = raze::options::functor<_Configurable_shuffle>;
constexpr inline auto clamp = raze::options::functor<_Configurable_clamp>;
constexpr inline auto sfence = raze::options::functor<_Configurable_sfence>;
constexpr inline auto fold = raze::options::functor<_Configurable_fold>;
constexpr inline auto vmin = vertical_min;
constexpr inline auto vmax = vertical_max;
constexpr inline auto hsum = horizontal_sum;
constexpr inline auto hmin = horizontal_min;
constexpr inline auto hmax = horizontal_max;


template <class T>
struct stream_ptr {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    pointer _ptr = nullptr;

    stream_ptr() noexcept {}
    explicit stream_ptr(pointer p) noexcept:  
        _ptr(p) 
    {}

    stream_ptr(const std::remove_const_t<T>* p) noexcept:
        _ptr(const_cast<pointer>(p)) 
    {}

    ~stream_ptr() noexcept {
        sfence();
    }

    raze_always_inline pointer get() const noexcept {
        return _ptr;
    }

    raze_always_inline stream_ptr operator+(std::ptrdiff_t offset) const noexcept {
        return stream_ptr(_ptr + offset);
    }

    raze_always_inline stream_ptr operator-(std::ptrdiff_t offset) const noexcept {
        return stream_ptr(_ptr - offset);
    }

    raze_always_inline stream_ptr& operator+=(std::ptrdiff_t offset) noexcept {
        _ptr += offset;
        return *this;
    }

    raze_always_inline stream_ptr& operator-=(std::ptrdiff_t offset) noexcept {
        _ptr -= offset;
        return *this;
    }

    raze_always_inline stream_ptr& operator++() noexcept {
        ++_ptr;
        return *this;
    }

    raze_always_inline stream_ptr operator++(int) noexcept {
        stream_ptr tmp = *this;
        ++_ptr;
        return tmp;
    }

    raze_always_inline bool operator==(const stream_ptr& other) const noexcept {
        return _ptr == other._ptr;
    }

    raze_always_inline bool operator!=(const stream_ptr& other) const noexcept {
        return _ptr != other._ptr;
    }

    raze_always_inline bool operator<(const stream_ptr& other) const noexcept {
        return _ptr < other._ptr;
    }

    raze_always_inline void advance_bytes(sizetype bytes) noexcept {
        algorithm::advance_bytes(_ptr, bytes);
    }
};

template <class T>
raze_always_inline constexpr stream_ptr<std::remove_cv_t<T>> stream(T* ptr) noexcept {
    return stream_ptr<std::remove_cv_t<T>>(ptr);
}

template <ternary_mask_expression_type Expression>
raze_always_inline constexpr auto as_ternary_mask() noexcept {
	return __as_ternary_mask<Expression>();
}

template <trivially_chunk_swappable V>
struct counter {
    using mask_type = typename V::mask_type;
    using value_type = typename V::value_type;
    using index_type = typename IntegerForSizeof<value_type>::Signed;
    using storage_type = std::conditional_t<(has_avx512f<abi_t<V>::isa> && sizeof(value_type) >= 4)
        || (has_avx512bw<abi_t<V>::isa>), sizetype, simd<index_type, abi_t<V>>>;

    counter() noexcept {
        if constexpr (simd_type<storage_type>) _storage = storage_type::zero();
        else _storage = 0;
    }

    raze_always_inline static constexpr auto portion_size() noexcept {
        return math::max_limit<index_type>();
    }

    raze_always_inline void count(const mask_type& mask) noexcept {
        if constexpr (std::integral<storage_type>) _storage += count_set(mask);
        else _storage.__for_each_chunk([&] <class _Chunk_, class _Chunk2_> (_Chunk_ & chunk, const _Chunk2_ & ch2) raze_always_inline_lambda {
            chunk = _Sub<abi_t<V>::isa, index_type>()(ustorage(chunk), ustorage(ch2));
        }, mask.__storage().storage());
    }

    raze_always_inline auto result() const noexcept {
        if constexpr (std::integral<storage_type>) return _storage;
        else return horizontal_sum(_storage);
    }

    storage_type _storage;
};

__RAZE_VX_NAMESPACE_END
