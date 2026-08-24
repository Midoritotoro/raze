#pragma once 

#include <src/raze/vx/storage/BestChunk.h>
#include <src/raze/vx/hw/x86/mask/SimdMaskTypeCheck.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi, u64 Elements, raw_mask_type M>
struct mask_wrapper {
    using unwrapped_type = M;
    using abi_type = Abi;
    using value_type = Type;

    static constexpr auto size = Elements;

    mask_wrapper(M mask) noexcept : _data(mask) {}

    template <intrin_type V> requires (sizeof(V) == sizeof(M))
    mask_wrapper(V v) noexcept : _data(as<M>(v)) {}

    mask_wrapper() noexcept = default;
    mask_wrapper(const mask_wrapper&) noexcept = default;
    mask_wrapper(mask_wrapper&&) noexcept = default;

    ~mask_wrapper() = default;

    mask_wrapper& operator=(const mask_wrapper&) noexcept = default;
    mask_wrapper& operator=(mask_wrapper&&) noexcept = default;

    raze_nodiscard raze_always_inline unwrapped_type data() const noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline unwrapped_type& data() noexcept {
        return _data;
    }
private:
    raze_no_unique_address unwrapped_type _data;
};

template <class T, class Abi, i32 Remaining_>
struct best_mask_chunk {
    static constexpr auto max_isa_width = has_avx512f<Abi::isa> ? 512 : has_avx<Abi::isa> 
        ? 256 : has_sse2<Abi::isa> ? 128 : 0;

    static constexpr auto bytes = Remaining * sizeof(T);
    static constexpr auto data_width = (bytes >= 64) ? 512 : (bytes >= 32) ? 256 : (bytes >= 16) ? 128 : 0;
    
    static constexpr auto width = (data_width < max_isa_width) ? data_width : max_isa_width;
    static constexpr auto use_kmask = (has_avx512f<Abi::isa> && width == 512 && sizeof(T) >= 4) ||
        (has_avx512bw<Abi::isa> && width == 512) || (has_avx512vl<Abi::isa> && sizeof(T) >= 4) ||
        (has_avx512bw<Abi::isa> && has_avx512vl<Abi::isa>);

    static constexpr auto fits = (width != 0);
    static constexpr auto elems_in_vector = (width == 0) ? Remaining : (width / 8) / sizeof(T);
    static constexpr auto elements_count = fits ? elems_in_vector : 1;

    using ChunkType = typename best_chunk<typename IntegerForSizeof<T>::Unsigned, Abi, Remaining>::type;

    using MaskIntrin = std::conditional_t<use_kmask && fits,
        mmask_for_elements_t<elems_in_vector>,
        std::conditional_t<fits, typename ChunkType::unwrapped_type, bool>>;

    using type = mask_wrapper<T, Abi, elements_count, MaskIntrin>;
};

template <class T, class Abi, i32 Remaining>
struct build_mask_tuple {
    using chunk_info = best_mask_chunk<T, Abi, Remaining>;
    using chunk = typename chunk_info::type;

    static constexpr auto elems = chunk_info::elements_count;
    static constexpr auto next = (Remaining > elems) ? Remaining - elems : 0;

    using type = simd_tuple_node<chunk, typename build_mask_tuple<T, Abi, next>::type>;
};

template <class T, class Abi>
struct build_mask_tuple<T, Abi, 0> {
    using type = simd_tuple_nil;
};

template <class T, class Abi>
using simd_mask_tuple_type = typename build_mask_tuple<T, Abi, Abi::size>::type;


__RAZE_VX_NAMESPACE_END
