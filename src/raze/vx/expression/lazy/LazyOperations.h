#pragma once 


#include <src/raze/vx/expression/lazy/AsTernaryMask.h>
#include <src/raze/vx/hw/x86/mask/MaskImplementation.h>
#include <src/raze/vx/hw/x86/arithmetic/direct/Fma.h>
#include <src/raze/vx/hw/x86/arithmetic/direct/Fms.h>
#include <src/raze/vx/hw/x86/arithmetic/direct/Fnms.h>
#include <src/raze/vx/hw/x86/arithmetic/direct/Fnma.h>


__RAZE_VX_NAMESPACE_BEGIN

template <class _Simd_>
struct _Fma_result {
    _Fma_result(
        const _Simd_& __x,
        const _Simd_& __y,
        const _Simd_& __z) noexcept:
            _x(__x), _y(__y), _z(__z) 
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Fma<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>()(
            __data(_x), __data(_y), __data(_z));
    }

    _Simd_ _x;
    _Simd_ _y;
    _Simd_ _z;
};

template <class _Simd_>
struct _Fms_result {
    _Fms_result(
        const _Simd_& __x,
        const _Simd_& __y,
        const _Simd_& __z) noexcept:
            _x(__x), _y(__y), _z(__z) 
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Fms<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>()(
            __data(_x), __data(_y), __data(_z));
    }

    _Simd_ _x;
    _Simd_ _y;
    _Simd_ _z;
};


template <class _Simd_>
struct _Fnma_result {
    _Fnma_result(
        const _Simd_& __x,
        const _Simd_& __y,
        const _Simd_& __z) noexcept :
            _x(__x), _y(__y), _z(__z)
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Fnma<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>()(
            __data(_x), __data(_y), __data(_z));
    }

    _Simd_ _x;
    _Simd_ _y;
    _Simd_ _z;
};

template <class _Simd_>
struct _Fnms_result {
    _Fnms_result(
        const _Simd_& __x,
        const _Simd_& __y,
        const _Simd_& __z) noexcept :
            _x(__x), _y(__y), _z(__z)
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Fnms<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>()(
            __data(_x), __data(_y), __data(_z));
    }

    _Simd_ _x;
    _Simd_ _y;
    _Simd_ _z;
};

template <class _Simd_>
struct _Add_result {
    _Add_result(
        const _Simd_& __x,
        const _Simd_& __y) noexcept :
            _x(__x), _y(__y)
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Add<_Simd_::__isa, _Simd_::__width, 
            typename _Simd_::value_type>()(__data(_x), __data(_y));
    }

    _Simd_ _x;
    _Simd_ _y;
};

template <class _Simd_>
struct _Mul_result {
    _Mul_result(
        const _Simd_& __x,
        const _Simd_& __y) noexcept :
        _x(__x), _y(__y)
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Mul<_Simd_::__isa, _Simd_::__width,
            typename _Simd_::value_type>()(__data(_x), __data(_y));
    }

    _Simd_ _x;
    _Simd_ _y;
};

template <class _Simd_>
struct _Sub_result {
    _Sub_result(
        const _Simd_& __x,
        const _Simd_& __y) noexcept :
            _x(__x), _y(__y)
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Sub<_Simd_::__isa, _Simd_::__width,
            typename _Simd_::value_type>()(__data(_x), __data(_y));
    }

    _Simd_ _x;
    _Simd_ _y;
};

template <class _Simd_>
struct _Negate_result {
    _Negate_result(const _Simd_& __x) noexcept:
        _x(__x)
    {}

    raze_always_inline operator _Simd_() const noexcept {
        return _Negate<_Simd_::__isa, _Simd_::__width,
            typename _Simd_::value_type>()(__data(_x));
    }

    _Simd_ _x;
};

__RAZE_VX_NAMESPACE_END
