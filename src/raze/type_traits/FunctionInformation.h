#pragma once 

#include <raze/Types.h>

#include <src/raze/type_traits/OperatorWrappers.h>

raze_disable_warning_msvc(4003)



__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

template <class _Type_>
_Type_ __returns_exactly() noexcept;

#define __EMPTY_ARGUMENT 

#define __EMIT_CDECL(function, opt1, opt2, opt3) function(__cdecl, opt1, opt2, opt3)

#if defined(_M_CEE)
#  define __EMIT_CLRCALL(function, opt1, opt2, opt3) function(__clrcall, opt1, opt2, opt3)
#else
#  define __EMIT_CLRCALL(function, opt1, opt2, opt3)
#endif // defined(_M_CEE)

#if defined(_M_IX86) && !defined(_M_CEE)
#define __EMIT_FASTCALL(function, opt1, opt2, opt3) function(__fastcall, opt1, opt2, opt3)
#else
#define __EMIT_FASTCALL(function, opt1, opt2, opt3)
#endif // defined(_M_IX86) && !defined(_M_CEE)

#if defined(_M_IX86)
#  define __EMIT_STDCALL(function, opt1, opt2, opt3)  function(__stdcall, opt1, opt2, opt3)
#  define __EMIT_THISCALL(function, opt1, opt2, opt3) function(__thiscall, opt1, opt2, opt3)
#else
#  define __EMIT_STDCALL(function, opt1, opt2, opt3)
#  define __EMIT_THISCALL(function, opt1, opt2, opt3)
#endif // defined(raze_processor_x86)

#if ((defined(_M_IX86) && _M_IX86_FP >= 2) || (defined(_M_X64) && !defined(_M_ARM64EC))) && !defined(_M_CEE)
#  define __EMIT_VECTORCALL(function, opt1, opt2, opt3) function(__vectorcall, opt1, opt2, opt3)
#else
#  define __EMIT_VECTORCALL(function, opt1, opt2, opt3)
#endif

#define __NON_MEMBER_CALL(func, cv_opt, ref_opt, noexcept_opt) \
    __EMIT_CDECL(func, cv_opt, ref_opt, noexcept_opt)          \
    __EMIT_CLRCALL(func, cv_opt, ref_opt, noexcept_opt)        \
    __EMIT_FASTCALL(func, cv_opt, ref_opt, noexcept_opt)       \
    __EMIT_STDCALL(func, cv_opt, ref_opt, noexcept_opt)        \
    __EMIT_VECTORCALL(func, cv_opt, ref_opt, noexcept_opt)

#define __NON_MEMBER_CALL_CV(func, ref_opt, noexcept_opt)    \
    __NON_MEMBER_CALL(func, , ref_opt, noexcept_opt)         \
    __NON_MEMBER_CALL(func, const, ref_opt, noexcept_opt)    \
    __NON_MEMBER_CALL(func, volatile, ref_opt, noexcept_opt) \
    __NON_MEMBER_CALL(func, const volatile, ref_opt, noexcept_opt)
    
#define __NON_MEMBER_CALL_CV_REF(func, noexcept_opt) \
    __NON_MEMBER_CALL_CV(func, , noexcept_opt)       \
    __NON_MEMBER_CALL_CV(func, &, noexcept_opt)      \
    __NON_MEMBER_CALL_CV(func, &&, noexcept_opt)

#if defined(__cpp_noexcept_function_type)   
#define __NON_MEMBER_CALL_CV_REF_NOEXCEPT(func) \
    __NON_MEMBER_CALL_CV_REF(func, )            \
    __NON_MEMBER_CALL_CV_REF(func, noexcept)
#else // ^^^ defined(__cpp_noexcept_function_type) / !defined(__cpp_noexcept_function_type) vvv
#define __NON_MEMBER_CALL_CV_REF_NOEXCEPT(func) _NON_MEMBER_CALL_CV_REF(func, )
#endif // ^^^ !defined(__cpp_noexcept_function_type) ^^^

#define __MEMBER_CALL(func, cv_opt, ref_opt, noexcept_opt) \
    __EMIT_CDECL(func, cv_opt, ref_opt, noexcept_opt)      \
    __EMIT_CLRCALL(func, cv_opt, ref_opt, noexcept_opt)    \
    __EMIT_FASTCALL(func, cv_opt, ref_opt, noexcept_opt)   \
    __EMIT_STDCALL(func, cv_opt, ref_opt, noexcept_opt)    \
    __EMIT_THISCALL(func, cv_opt, ref_opt, noexcept_opt)   \
    __EMIT_VECTORCALL(func, cv_opt, ref_opt, noexcept_opt)

#define __MEMBER_CALL_CV(func, ref_opt, noexcept_opt)    \
    __MEMBER_CALL(func, , ref_opt, noexcept_opt)         \
    __MEMBER_CALL(func, const, ref_opt, noexcept_opt)    \
    __MEMBER_CALL(func, volatile, ref_opt, noexcept_opt) \
    __MEMBER_CALL(func, const volatile, ref_opt, noexcept_opt)

#define __MEMBER_CALL_CV_REF(func, noexcept_opt) \
    __MEMBER_CALL_CV(func, , noexcept_opt)       \
    __MEMBER_CALL_CV(func, &, noexcept_opt)      \
    __MEMBER_CALL_CV(func, &&, noexcept_opt)

#if defined(__cpp_noexcept_function_type)
#  define __MEMBER_CALL_CV_REF_NOEXCEPT(func) \
      __MEMBER_CALL_CV_REF(func, )            \
      __MEMBER_CALL_CV_REF(func, noexcept)
#else
#  define __MEMBER_CALL_CV_REF_NOEXCEPT(func) __MEMBER_CALL_CV_REF(func, )
#endif // defined(__cpp_noexcept_function_type)

#if defined(__cpp_noexcept_function_type)
#define __CLASS_DEFINE_CV_REF_NOEXCEPT(CLASS) \
        CLASS(__EMPTY_ARGUMENT)                   \
        CLASS(const)                             \
        CLASS(volatile)                          \
        CLASS(const volatile)                    \
        CLASS(&)                                 \
        CLASS(const&)                            \
        CLASS(volatile&)                         \
        CLASS(const volatile&)                   \
        CLASS(&&)                                \
        CLASS(const&&)                           \
        CLASS(volatile&&)                        \
        CLASS(const volatile&&)                  \
        CLASS(noexcept)                          \
        CLASS(const noexcept)                    \
        CLASS(volatile noexcept)                 \
        CLASS(const volatile noexcept)           \
        CLASS(& noexcept)                        \
        CLASS(const& noexcept)                   \
        CLASS(volatile& noexcept)                \
        CLASS(const volatile& noexcept)          \
        CLASS(&& noexcept)                       \
        CLASS(const&& noexcept)                  \
        CLASS(volatile&& noexcept)               \
        CLASS(const volatile&& noexcept)
#else
#define __CLASS_DEFINE_CV_REF_NOEXCEPT(CLASS) \
        CLASS(__EMPTY_ARGUMENT)                   \
        CLASS(const)                             \
        CLASS(volatile)                          \
        CLASS(const volatile)                    \
        CLASS(&)                                 \
        CLASS(const&)                            \
        CLASS(volatile&)                         \
        CLASS(const volatile&)                   \
        CLASS(&&)                                \
        CLASS(const&&)                           \
        CLASS(volatile&&)                        \
        CLASS(const volatile&&)
#endif // defined(__cpp_noexcept_function_type)

template <class _Type_>
struct __function_information {
    using is_member_function_pointer = std::false_type;
};

#define __IS_MEMBER_FUNCTION_POINTER(call_opt, cv_opt, ref_opt, noexcept_opt)                            \
    template <class _Return_, class _FirstArgument_, class... _Args_>                                   \
    struct __function_information<_Return_ (call_opt _FirstArgument_::*)(_Args_...) cv_opt ref_opt noexcept_opt> { \
        using is_member_function_pointer = std::true_type;            \
        using return_type = _Return_;                 \
        using class_type = _FirstArgument_;                \
    };

__MEMBER_CALL_CV_REF_NOEXCEPT(__IS_MEMBER_FUNCTION_POINTER)
#undef __IS_MEMBER_FUNCTION_POINTER

#define __IS_MEMBER_FUNCTION_POINTER_ELLIPSIS(cv_ref_noexcept_opt) \
    template <class _Return_, class _FirstArgument_, class... _Args_> \
    struct __function_information<_Return_ (_FirstArgument_::*)(_Args_..., ...) cv_ref_noexcept_opt> { \
        using is_member_function_pointer = std::true_type; \
        using return_type = _Return_; \
        using class_type = _FirstArgument_; \
    };

__CLASS_DEFINE_CV_REF_NOEXCEPT(__IS_MEMBER_FUNCTION_POINTER_ELLIPSIS)
#undef __IS_MEMBER_FUNCTION_POINTER_ELLIPSIS

template <
    class                       _Type_,
    template <class...> class   _Template_>
constexpr bool is_specialization_v = false;

template <
    template <class...> class   _Template_,
    class...                    _Types_>
constexpr bool is_specialization_v<_Template_<_Types_...>, _Template_> = true;

template <class>
struct __member_object_pointer_class_type {};

template <
    class _FirstType_,
    class _SecondType_>
struct __member_object_pointer_class_type<_FirstType_ _SecondType_::*> {
    using type = _SecondType_;
};

template <class ... _Types_>
using function_class_type = typename __function_information<_Types_...>::class_type;

template <class ... _Types_>
constexpr inline bool is_member_function_pointer_v = __function_information<_Types_...>::is_member_function_pointer::value;

__RAZE_TYPE_TRAITS_NAMESPACE_END
