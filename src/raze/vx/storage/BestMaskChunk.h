#pragma once 

#include <src/raze/vx/storage/BestChunk.h>

__RAZE_VX_NAMESPACE_BEGIN

template <int _N_>
struct __mask_type;

template <> struct __mask_type<2>  { using type = __mmask8;  };
template <> struct __mask_type<4>  { using type = __mmask8;  };
template <> struct __mask_type<8>  { using type = __mmask8;  };
template <> struct __mask_type<16> { using type = __mmask16; };
template <> struct __mask_type<32> { using type = __mmask32; };
template <> struct __mask_type<64> { using type = __mmask64; };

template <class _Type_, class _Abi_, int _Remaining_>
struct best_mask_chunk {
    static constexpr int __max_isa_width =
        __has_avx512f_support_v<_Abi_::isa> ? 512 :
        __has_avx_support_v<_Abi_::isa> ? 256 :
        __has_sse2_support_v<_Abi_::isa> ? 128 : 0;

    static constexpr int __bytes = _Remaining_ * sizeof(_Type_);

    static constexpr int __width =
        (__bytes >= 64 && __max_isa_width >= 512) ? 512 :
        (__bytes >= 32 && __max_isa_width >= 256) ? 256 :
        (__bytes >= 16 && __max_isa_width >= 128) ? 128 : 0;

    static constexpr int __elems = (__width == 0) ? 1 : (__width / 8) / sizeof(_Type_);
    static constexpr bool __use_kmask = __has_avx512f_support_v<_Abi_::isa> && __elems <= 64;

    static constexpr int __mask_bits =
        (__elems <= 2) ? 2 :
        (__elems <= 4) ? 4 :
        (__elems <= 8) ? 8 :
        (__elems <= 16) ? 16 :
        (__elems <= 32) ? 32 : 64;

    using type = std::conditional_t<
        __use_kmask,
        typename __mask_type<__mask_bits>::type,
        typename best_chunk<_Type_, _Abi_, _Remaining_>::type>;
};

template <class _Type_, class _Abi_, int _Remaining_>
struct __build_mask_tuple {
    using __chunk = typename best_mask_chunk<_Type_, _Abi_, _Remaining_>::type;

    static constexpr int __elems =
        std::is_integral_v<__chunk> ?
            (__has_avx512f_support_v<_Abi_::isa>
                ? (
                    (sizeof(__chunk) == 1) ? 8 :
                    (sizeof(__chunk) == 2) ? 16 :
                    (sizeof(__chunk) == 4) ? 32 : 64
                  )
                : sizeof(__chunk) * 8)
            : sizeof(__chunk) / sizeof(_Type_);

    static constexpr int __next = (_Remaining_ > __elems) ? _Remaining_ - __elems : 0;
    using type = _Simd_tuple_node<__chunk, typename __build_mask_tuple<_Type_, _Abi_, __next>::type>;
};

template <class _Type_, class _Abi_>
struct __build_mask_tuple<_Type_, _Abi_, 0> {
    using type = _Simd_tuple_nil;
};

template <class _Type_, class _Abi_>
using _Simd_mask_tuple_type = typename __build_mask_tuple<_Type_, _Abi_, _Abi_::size>::type;

__RAZE_VX_NAMESPACE_END