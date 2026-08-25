#pragma once 

#include <src/raze/vx/storage/Size.h>
#include <src/raze/vx/hw/x86/access/Insert.h>
#include <src/raze/vx/hw/x86/access/Extract.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class T, class Abi, u64 Elements, class V>
struct vector_wrapper {
    using unwrapped_type = V;
    using abi_type = Abi;
    using value_type = T;
    using as_simd = simd<T, resize_abi_t<Abi, Elements>>;

    static constexpr auto size = Elements;

    template <intrin_type V2> requires (sizeof(V) == sizeof(V2))
    vector_wrapper(V2 v) noexcept : _data(as<V>(v))
    {}

    vector_wrapper() noexcept = default;

    template <sizetype I, intrin_type V2>
    raze_always_inline void insert(V2 v2) noexcept {
        insert_vector_<Abi::isa>(_data, std::integral_constant<sizetype, I>{}, v2);
    }

    template <sizetype I, intrin_type V2>
    raze_always_inline V2 extract() const noexcept {
        return extract_vector_<V2, Abi::isa>(_data, std::integral_constant<sizetype, I>{});
    }

    raze_nodiscard raze_always_inline unwrapped_type data() const noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline unwrapped_type& data() noexcept {
        return _data;
    }
private:
    raze_no_unique_address unwrapped_type _data;
};

template <class T, class Abi>
struct scalar_wrapper {
    using unwrapped_type = T;
    using abi_type = Abi;
    using value_type = T;
    using as_simd = simd<T, resize_abi_t<Abi, 1>>;

    static constexpr auto size = 1;

    scalar_wrapper(T v) noexcept : _data(v) {}
    scalar_wrapper() noexcept = default;

    raze_nodiscard raze_always_inline unwrapped_type data() const noexcept {
        return _data;
    }

    raze_nodiscard raze_always_inline unwrapped_type& data() noexcept {
        return _data;
    }
private:
    raze_no_unique_address unwrapped_type _data;
};

template <class T, class Abi, i32 Remaining>
struct best_chunk {
    static constexpr auto total_bytes = Remaining * sizeof(T);
    static constexpr auto max_isa_width = has_avx512f<Abi::isa> ? 512 :
        has_avx<Abi::isa> ? 256 : has_sse2<Abi::isa> ? 128 : 0;

    static constexpr auto data_width = (total_bytes >= 64) ? 512 :
        (total_bytes >= 32) ? 256 : (total_bytes >= 16) ? 128 : 0;

    static constexpr auto width = (data_width < max_isa_width) ? data_width : max_isa_width;
    static constexpr auto length = width / raze_sizeof_in_bits(T);

    using intrin = traits::deduce_simd_vector_type<T, width>;
    using type = std::conditional_t<width != 0 && intrin_type<intrin>,
        vector_wrapper<T, Abi, length, intrin>, scalar_wrapper<T, Abi>>;
};

template <class T, class Abi, i32 Remaining>
struct build_tuple {
    using chunk_type = typename best_chunk<T, Abi, Remaining>::type;

    static constexpr auto chunk_elems = sizeof(chunk_type) / sizeof(T);
    static constexpr auto next = (Remaining > chunk_elems) ? (Remaining - chunk_elems) : 0;
    
    using type = simd_tuple_node<chunk_type, typename build_tuple<T, Abi, next>::type>;
};

template <class T, class Abi> 
struct build_tuple<T, Abi, 0> {
    using type = simd_tuple_nil; 
};

template <class T, class Abi>
using simd_vector_tuple_type = typename build_tuple<T, Abi, Abi::size>::type;

__RAZE_VX_NAMESPACE_END
