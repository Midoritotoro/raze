#pragma once 

#include <raze/compatibility/Compatibility.h>
#include <src/raze/type_traits/Invoke.h>

#include <src/raze/datapar/SimdMaskTypeCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Derived_>
struct __simd_mask_common_operations {
    using __self			= std::remove_cvref_t<_Derived_>;

    static constexpr raze_always_inline arch::ISA __isa() noexcept;
    static constexpr raze_always_inline uint8 __bit_width() noexcept;
    constexpr raze_always_inline auto __unwrap() const noexcept;

    constexpr raze_always_inline bool all_of() const noexcept;
    constexpr raze_always_inline bool any_of() const noexcept;
    constexpr raze_always_inline bool none_of() const noexcept;

    constexpr raze_always_inline __self operator&(const __self& __other) const noexcept;
    constexpr raze_always_inline __self operator|(const __self& __other) const noexcept;
    constexpr raze_always_inline __self operator^(const __self& __other) const noexcept;

    constexpr raze_always_inline __self operator>>(const uint8 __shift) const noexcept;
    constexpr raze_always_inline __self operator<<(const uint8 __shift) const noexcept;
    
    template <uint8 _Shift_>
    constexpr raze_always_inline __self operator>>(const std::integral_constant<uint8, _Shift_> __shift) const noexcept;

    template <uint8 _Shift_>
    constexpr raze_always_inline __self operator<<(const std::integral_constant<uint8, _Shift_> __shift) const noexcept;

    constexpr raze_always_inline __self operator~() const noexcept;

    constexpr raze_always_inline auto __to_kmask() const noexcept;
    constexpr raze_always_inline auto __to_int() const noexcept;

    constexpr raze_always_inline explicit operator bool() const noexcept;

    constexpr raze_always_inline bool operator==(const __self& __other) const noexcept;
    constexpr raze_always_inline bool operator!=(const __self& __other) const noexcept;
};

__RAZE_DATAPAR_NAMESPACE_END

#include <src/raze/datapar/SimdMaskCommonOperations.inl>
