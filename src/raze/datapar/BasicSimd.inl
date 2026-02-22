#pragma once 

raze_disable_warning_msvc(4002)

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_>::simd() noexcept
{}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
template <
    typename _VectorType_,
    std::enable_if_t<__is_intrin_type_v<_VectorType_> || __is_valid_simd_v<_VectorType_>, int>>
simd<_ISA_, _Type_, _Width_>::simd(_VectorType_ __other) noexcept {
    _vector = simd_cast<vector_type>(__other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_>::simd(value_type __value) noexcept {
    fill(__value);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_>::~simd() noexcept
{}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline bool simd<_ISA_, _Type_, _Width_>::is_supported() noexcept {
    return arch::ProcessorFeatures::isSupported<_ISA_>();
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_> simd<_ISA_, _Type_, _Width_>::zero() noexcept {
    return _Simd_broadcast_zeros<_ISA_, _Width_, vector_type>()();
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>& 
    simd<_ISA_, _Type_, _Width_>::fill(value_type __value) noexcept
{
    _vector = _Simd_broadcast<_ISA_, _Width_, vector_type>()(__value);
    return *this;
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
    simd<_ISA_, _Type_, _Width_>::operator+=(const simd& __other) noexcept
{
    return *this = (*this + __other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
    simd<_ISA_, _Type_, _Width_>::operator-=(const simd& __other) noexcept
{
    return *this = (*this - __other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
    simd<_ISA_, _Type_, _Width_>::operator*=(const simd& __other) noexcept
{
    return *this = (*this * __other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
    simd<_ISA_, _Type_, _Width_>::operator/=(const simd& __other) noexcept
{
    return *this = (*this / __other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
    simd<_ISA_, _Type_, _Width_>::operator=(const simd& __left) noexcept
{
    _vector = __left._vector;
    return *this;
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline _Type_
    simd<_ISA_, _Type_, _Width_>::operator[](const size_type __index) const noexcept 
{
    return _Simd_extract<_ISA_, _Width_, _Type_>()(_vector, __index);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd_element_reference<simd<_ISA_, _Type_, _Width_>>
    simd<_ISA_, _Type_, _Width_>::operator[](const size_type __index) noexcept
{
    return simd_element_reference<simd>(this, __index);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>
simd<_ISA_, _Type_, _Width_>::operator++(int) noexcept {
    simd __self = *this;
    *this += simd(1);
    return __self;
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
simd<_ISA_, _Type_, _Width_>::operator++() noexcept {
    return *this += simd(1);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>
simd<_ISA_, _Type_, _Width_>::operator--(int) noexcept
{
    simd __self = *this;
    *this -= simd(1);
    return __self;
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
simd<_ISA_, _Type_, _Width_>::operator--() noexcept
{
    return *this -= simd(1);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>
simd<_ISA_, _Type_, _Width_>::operator~() const noexcept {
    return _Simd_bit_not<_ISA_, _Width_>()(_vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
simd<_ISA_, _Type_, _Width_>::operator&=(const simd& __other) noexcept {
    return *this = (*this & __other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
simd<_ISA_, _Type_, _Width_>::operator|=(const simd& __other) noexcept {
    return *this = (*this | __other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>&
simd<_ISA_, _Type_, _Width_>::operator^=(const simd& __other) noexcept {
    return *this = (*this ^ __other);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator/(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return {}; // return _Simd_divide<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator+(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_add<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator-(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_sub<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator*(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return {}; // return _Simd_multiply<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator&(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_bit_and<_ISA_, _Width_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator|(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_bit_or<_ISA_, _Width_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator^(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_bit_xor<_ISA_, _Width_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator+(
    const simd<_ISA_, _Type_, _Width_>&                      __left,
    const typename simd<_ISA_, _Type_, _Width_>::value_type  __right) noexcept
{
    return __left + simd(__right);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator-(
    const simd<_ISA_, _Type_, _Width_>&                      __left,
    const typename simd<_ISA_, _Type_, _Width_>::value_type  __right) noexcept
{
    return _Simd_substract<_ISA_, _Width_, _Type_>()(__left._vector, __simd_unwrap(simd<_ISA_, _Type_, _Width_>(__right)));
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator*(
    const simd<_ISA_, _Type_, _Width_>&                      __left,
    const typename simd<_ISA_, _Type_, _Width_>::value_type  __right) noexcept
{
    return {}; // return _Simd_multiply<_ISA_, _Width_, _Type_>()(__left._vector, __simd_unwrap(simd<_ISA_, _Type_, _Width_>(__right)));
}


template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
simd<_ISA_, _Type_, _Width_> operator/(
    const simd<_ISA_, _Type_, _Width_>&                      __left,
    const typename simd<_ISA_, _Type_, _Width_>::value_type  __right) noexcept
{
    return {}; // return __simd_divide<_ISA_, _Width_, _Type_>(__left._vector, __simd_unwrap(simd<_ISA_, _Type_, _Width_>(__right)));
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>
    simd<_ISA_, _Type_, _Width_>::operator+() const noexcept 
{
    return _vector;
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>
    simd<_ISA_, _Type_, _Width_>::operator-() const noexcept
{
    return __simd_negate<_ISA_, _Width_, _Type_>(_vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline _Type_ simd<_ISA_, _Type_, _Width_>::extract(size_type __index) const noexcept
{
    return _Simd_extract<_ISA_, _Width_, value_type>()(_vector, __index);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd_element_reference<simd<_ISA_, _Type_, _Width_>>
    simd<_ISA_, _Type_, _Width_>::extract_wrapped(size_type __index) noexcept
{
    return simd_element_reference<simd>(this, __index);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline void simd<_ISA_, _Type_, _Width_>::insert(
    size_type   __position,
    value_type  __value) noexcept
{
    _Simd_insert<_ISA_, _Width_>()(_vector, __position, __value);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
typename simd<_ISA_, _Type_, _Width_>::__simd_native_compare_return_type operator==(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
typename simd<_ISA_, _Type_, _Width_>::__simd_native_compare_return_type operator!=(
    const simd<_ISA_, _Type_, _Width_>& __left, 
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_not_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
typename simd<_ISA_, _Type_, _Width_>::__simd_native_compare_return_type operator<(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_less<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
typename simd<_ISA_, _Type_, _Width_>::__simd_native_compare_return_type operator<=(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_less_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
typename simd<_ISA_, _Type_, _Width_>::__simd_native_compare_return_type operator>(
    const simd<_ISA_, _Type_, _Width_>& __left, 
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_greater<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
typename simd<_ISA_, _Type_, _Width_>::__simd_native_compare_return_type operator>=(
    const simd<_ISA_, _Type_, _Width_>& __left,
    const simd<_ISA_, _Type_, _Width_>& __right) noexcept
{
    return _Simd_greater_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
template <typename _DesiredType_>
raze_always_inline simd_mask<_ISA_, _DesiredType_, _Width_>
    simd<_ISA_, _Type_, _Width_>::to_mask() const noexcept
{
    return _Simd_to_mask<_ISA_, _Width_, _DesiredType_>()(_vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
template <typename _DesiredType_>
raze_always_inline auto simd<_ISA_, _Type_, _Width_>::to_index_mask() const noexcept {
    return _Simd_to_index_mask<_ISA_, _Width_, _DesiredType_>()(_vector);
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
constexpr int simd<_ISA_, _Type_, _Width_>::width() noexcept {
    return _Width_;
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
constexpr int simd<_ISA_, _Type_, _Width_>::size() noexcept {
    return (sizeof(vector_type) / sizeof(_Type_));
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
constexpr int simd<_ISA_, _Type_, _Width_>::length() noexcept {
    return size();
}

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_>
raze_always_inline simd<_ISA_, _Type_, _Width_>::vector_type
simd<_ISA_, _Type_, _Width_>::unwrap() const noexcept
{
    return _vector;
}

__RAZE_DATAPAR_NAMESPACE_END
