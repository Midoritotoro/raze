#pragma once 

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    typename _Simd_,
    typename _ImposedElementType_>
simd_element_reference<_Simd_, _ImposedElementType_>::simd_element_reference(
    parent_type*    __parent,
    uint32          __index
) noexcept:
    _parent(__parent),
    _index(__index)
{
    raze_debug_assert(__parent != nullptr);
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
simd_element_reference<_Simd_, _ImposedElementType_>::~simd_element_reference() noexcept
{}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline uint32 simd_element_reference<_Simd_, _ImposedElementType_>::index() const noexcept {
    return _index;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>::value_type
    simd_element_reference<_Simd_, _ImposedElementType_>::get() const noexcept 
{
    return _parent->extract(_index);
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline void simd_element_reference<_Simd_, _ImposedElementType_>::set(value_type __value) noexcept {
    _parent->insert(_index, __value);
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator=(const value_type __other) noexcept 
{
    set(__other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator++() noexcept 
{
    set(get() + 1);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator--() noexcept
{
    set(get() - 1);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>
    simd_element_reference<_Simd_, _ImposedElementType_>::operator++(int) noexcept 
{
    auto __self = *this;
    set(get() + 1);
    return __self;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>
    simd_element_reference<_Simd_, _ImposedElementType_>::operator--(int) noexcept 
{
    auto __self = *this;
    set(get() - 1);
    return __self;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>::value_type
    simd_element_reference<_Simd_, _ImposedElementType_>::operator-() noexcept
{
    return -get();
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>::value_type
    simd_element_reference<_Simd_, _ImposedElementType_>::operator+() noexcept 
{
    return get();
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>::operator value_type() const noexcept {
    return get();
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline bool simd_element_reference<_Simd_, _ImposedElementType_>
    ::operator==(const value_type __other) const noexcept
{
    return get() == __other;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline bool simd_element_reference<_Simd_, _ImposedElementType_>
    ::operator!=(const value_type __other) const noexcept
{
    return get() != __other;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline bool simd_element_reference<_Simd_, _ImposedElementType_>
    ::operator>(const value_type __other) const noexcept
{
    return get() > __other;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline bool simd_element_reference<_Simd_, _ImposedElementType_>
    ::operator<(const value_type __other) const noexcept
{
    return get() < __other;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline bool simd_element_reference<_Simd_, _ImposedElementType_>
    ::operator<=(const value_type __other) const noexcept
{
    return get() <= __other;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline bool simd_element_reference<_Simd_, _ImposedElementType_>
    ::operator>=(const value_type __other) const noexcept
{
    return get() >= __other;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator+=(const value_type __other) noexcept
{
    set(get() + __other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator-=(const value_type __other) noexcept
{
    set(get() - __other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>&
    simd_element_reference<_Simd_, _ImposedElementType_>::operator*=(const value_type __other) noexcept
{
    set(get() * __other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator/=(const value_type __other) noexcept
{
    set(get() / __other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator%=(const value_type __other) noexcept
{
    set(get() % __other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>& 
    simd_element_reference<_Simd_, _ImposedElementType_>::operator&=(const value_type __other) noexcept
{
    set(get() & __other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>&
    simd_element_reference<_Simd_, _ImposedElementType_>::operator^=(const value_type __other) noexcept
{
    set(get() ^ __other);
    return *this;
}

template <
    typename _Simd_,
    typename _ImposedElementType_>
raze_always_inline simd_element_reference<_Simd_, _ImposedElementType_>&
    simd_element_reference<_Simd_, _ImposedElementType_>::operator|=(const value_type __other) noexcept
{
    set(get() | __other);
    return *this;
}

__RAZE_DATAPAR_NAMESPACE_END
