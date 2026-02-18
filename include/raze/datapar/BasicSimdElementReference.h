#pragma once 

#include <src/raze/type_traits/SimdTypeCheck.h>
#include <src/raze/type_traits/IsVirtualBaseOf.h>

#include <raze/compatibility/Inline.h>
#include <src/raze/utility/Assert.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    typename _Simd_, 
    typename _ImposedElementType_ = typename _Simd_::value_type>
class simd_element_reference {
    static_assert(__is_valid_simd_v<_Simd_>);
public: 
    using parent_type   = _Simd_;
    using vector_type   = typename _Simd_::vector_type;
    using value_type    = _ImposedElementType_;

    simd_element_reference(
        parent_type*    __parent,
        uint32          __index = 0) noexcept;

    ~simd_element_reference() noexcept;

    raze_always_inline uint32 index() const noexcept;
    raze_always_inline value_type get() const noexcept;
    raze_always_inline void set(value_type value) noexcept;

    raze_always_inline simd_element_reference& operator=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator++() noexcept;
    raze_always_inline simd_element_reference& operator--() noexcept;
    raze_always_inline simd_element_reference operator++(int) noexcept;
    raze_always_inline simd_element_reference operator--(int) noexcept;

    raze_always_inline value_type operator-() noexcept;
    raze_always_inline value_type operator+() noexcept;
    raze_always_inline operator value_type() const noexcept;

    raze_always_inline bool operator==(const value_type __other) const noexcept;
    raze_always_inline bool operator!=(const value_type __other) const noexcept;
    raze_always_inline bool operator>(const value_type __other) const noexcept;
    raze_always_inline bool operator<(const value_type __other) const noexcept;
    raze_always_inline bool operator<=(const value_type __other) const noexcept;
    raze_always_inline bool operator>=(const value_type __other) const noexcept;

    raze_always_inline simd_element_reference& operator+=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator-=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator*=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator/=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator%=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator&=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator^=(const value_type __other) noexcept;
    raze_always_inline simd_element_reference& operator|=(const value_type __other) noexcept;
private:
    parent_type*    _parent = nullptr;
    uint32          _index  = 0;
};

__RAZE_DATAPAR_NAMESPACE_END

#include <src/raze/datapar/BasicSimdElementReference.inl>