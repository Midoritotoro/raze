#pragma once 

#include <raze/Types.h>
#include <src/raze/type_traits/FunctionInformation.h>


__RAZE_TYPE_TRAITS_NAMESPACE_BEGIN

enum class __invoker_strategy : uint8 {
    __functor,                              // invoke(_FunctorObject, _Args...);

    __ptr_to_member_function_with_obj,      // invoke(&_ObjectType::_MemberFunction, _Object, _Args...);
    __ptr_to_member_function_with_refwrap,  // invoke(&_ObjectType::_MemberFunction, std::ref(_Object), _Args...);
    __ptr_to_member_function_with_ptr,      // invoke(&_ObjectType::_MemberFunction, _ObjectPointer, _Args...);
    
    __ptr_to_member_data_with_obj,          // invoke(&_ObjectType::_MemberData, _Object, _Args...);
    __ptr_to_member_data_with_refwrap,      // invoke(&_ObjectType::_MemberData, std::ref(_Object), _Args...);
    __ptr_to_member_data_with_ptr           // invoke(&_ObjectType::_MemberData, _ObjectPointer, _Args...);
};

template <__invoker_strategy _Strategy_>
struct __invoker;

template <>
struct __invoker<__invoker_strategy::__functor> {
    static constexpr auto __strategy = __invoker_strategy::__functor;

    template <
        class       _Callable_,
        class...    _Args_>
    inline static constexpr auto __call(
        _Callable_&& __object,
        _Args_&&...  __args)
        noexcept(noexcept(static_cast<_Callable_&&>(__object)(static_cast<_Args_&&>(__args)...)))
        -> decltype(static_cast<_Callable_&&>(__object)(static_cast<_Args_&&>(__args)...))
    {
        return static_cast<_Callable_&&>(__object)(static_cast<_Args_&&>(__args)...);
    }
};

template <>
struct __invoker<__invoker_strategy::__ptr_to_member_function_with_obj> {
    static constexpr auto __strategy = __invoker_strategy::__ptr_to_member_function_with_obj;

    template <
        class       _MemberFunction_,
        class       _Object_,
        class...    _Args_>
    inline static constexpr auto __call(
        _MemberFunction_    __member_function,
        _Object_&& __object,
        _Args_&&...         __args)
        noexcept(noexcept((static_cast<_Object_&&>(__object).*__member_function)(static_cast<_Args_&&>(__args)...)))
        -> decltype((static_cast<_Object_&&>(__object).*__member_function)(static_cast<_Args_&&>(__args)...))
    {
        return (static_cast<_Object_&&>(__object).*__member_function)(static_cast<_Args_&&>(__args)...);
    }
};

template <>
struct __invoker<__invoker_strategy::__ptr_to_member_function_with_refwrap> {
    static constexpr auto __strategy = __invoker_strategy::__ptr_to_member_function_with_refwrap;

    template <
        class       _MemberFunction_,
        class       _ReferenceWrapper_,
        class...    _Args_>
    inline static constexpr auto __call(
        _MemberFunction_    __member_function,
        _ReferenceWrapper_  __reference_wrapper,
        _Args_&&...         __args)
        noexcept(noexcept((__reference_wrapper.get().*__member_function)(static_cast<_Args_&&>(__args)...)))
        -> decltype((__reference_wrapper.get().*__member_function)(static_cast<_Args_&&>(__args)...))
    {
        return (__reference_wrapper.get().*__member_function)(static_cast<_Args_&&>(__args)...);
    }
};

template <>
struct __invoker<__invoker_strategy::__ptr_to_member_function_with_ptr> {
    static constexpr auto __strategy = __invoker_strategy::__ptr_to_member_function_with_ptr;

    template <
        class       _MemberFunction_,
        class       _Pointer_,
        class...    _Args_>
    inline static constexpr auto __call(
        _MemberFunction_    __member_function,
        _Pointer_&& __pointer_to_object,
        _Args_&&...         __args)
        noexcept(noexcept(((*static_cast<_Pointer_&&>(__pointer_to_object)).*__member_function)(static_cast<_Args_&&>(__args)...)))
        -> decltype(((*static_cast<_Pointer_&&>(__pointer_to_object)).*__member_function)(static_cast<_Args_&&>(__args)...))
    {
        return ((*static_cast<_Pointer_&&>(__pointer_to_object)).*__member_function)(static_cast<_Args_&&>(__args)...);
    }
};


template <>
struct __invoker<__invoker_strategy::__ptr_to_member_data_with_obj> {
    static constexpr auto __strategy = __invoker_strategy::__ptr_to_member_data_with_obj;

    template <
        class _MemberData_,
        class _Object_>
    inline static constexpr auto __call(
        _MemberData_    __member_data,
        _Object_&& __object) noexcept -> decltype(static_cast<_Object_&&>(__object).*__member_data)
    {
        return static_cast<_Object_&&>(__object).*__member_data;
    }
};

template <>
struct __invoker<__invoker_strategy::__ptr_to_member_data_with_refwrap> {
    static constexpr auto __strategy = __invoker_strategy::__ptr_to_member_data_with_refwrap;

    template <
        class _MemberData_,
        class _ReferenceWrapper_>
    inline static constexpr auto __call(
        _MemberData_        __member_data,
        _ReferenceWrapper_  __reference_wrapper) noexcept -> decltype(__reference_wrapper.get().*__member_data)
    {
        return __reference_wrapper.get().*__member_data;
    }
};

template <>
struct __invoker<__invoker_strategy::__ptr_to_member_data_with_ptr> {
    static constexpr auto __strategy = __invoker_strategy::__ptr_to_member_data_with_ptr;

    template <
        class _MemberData_,
        class _Pointer_>
    inline static constexpr auto __call(
        _MemberData_    __member_data,
        _Pointer_&&     __pointer_to_object) noexcept(noexcept((*static_cast<_Pointer_&&>(__pointer_to_object)).*__member_data))
        -> decltype((*static_cast<_Pointer_&&>(__pointer_to_object)).*__member_data)
    {
        return (*static_cast<_Pointer_&&>(__pointer_to_object)).*__member_data;
    }
};

template <
    class _Callable_,
    class _Object_,
    class _RemovedQualifiers_ = std::remove_cvref_t<_Callable_>,
    bool _IsMemberFunctionPointer_ = std::is_member_function_pointer_v<_RemovedQualifiers_>,
    bool _IsMemberFunctionData_ = std::is_member_object_pointer_v<_RemovedQualifiers_>>
struct __select_invoker;

template <
    class _Callable_,
    class _Object_,
    class _RemovedQualifiers_>
struct __select_invoker<_Callable_, _Object_, _RemovedQualifiers_, true, false> {
    using type = std::conditional_t<
        std::is_same_v<function_class_type<_RemovedQualifiers_>, std::remove_cvref_t<_Object_>> ||
        std::is_base_of_v<function_class_type<_RemovedQualifiers_>, std::remove_cvref_t<_Object_>>,
        __invoker<__invoker_strategy::__ptr_to_member_function_with_obj>,
        std::conditional_t<
        type_traits::is_specialization_v<std::remove_cvref_t<_Object_>, std::reference_wrapper>,
        __invoker<__invoker_strategy::__ptr_to_member_function_with_refwrap>,
        __invoker<__invoker_strategy::__ptr_to_member_function_with_ptr>>>;
};

template <
    class _Callable_,
    class _Object_,
    class _RemovedQualifiers_>
struct __select_invoker<_Callable_, _Object_, _RemovedQualifiers_, false, true> {
    using type = std::conditional_t<
        std::is_same_v<typename __member_object_pointer_class_type<_RemovedQualifiers_>::type, std::remove_cvref_t<_Object_>> ||
        std::is_base_of_v<typename __member_object_pointer_class_type<_RemovedQualifiers_>::type, std::remove_cvref_t<_Object_>>,
        __invoker<__invoker_strategy::__ptr_to_member_data_with_obj>,
        std::conditional_t<
        type_traits::is_specialization_v<std::remove_cvref_t<_Object_>, std::reference_wrapper>,
        __invoker<__invoker_strategy::__ptr_to_member_data_with_refwrap>,
        __invoker<__invoker_strategy::__ptr_to_member_data_with_ptr>>>;
};

template <
    class _Callable_,
    class _Object_,
    class _RemovedQualifiers_>
struct __select_invoker<_Callable_, _Object_, _RemovedQualifiers_, false, false> {
    using type = __invoker<__invoker_strategy::__functor>;
};

template <
    class _Callable_,
    class _Object_>
using __invoker_type = typename __select_invoker<_Callable_, _Object_>::type;

template <
    class _From_,
    class _To_,
    class = void>
struct __invoke_convertible :
    std::false_type
{
};

template <
    class _From_,
    class _To_>
struct __invoke_convertible<_From_, _To_, std::void_t<decltype(__fake_copy_init<_To_>(__returns_exactly<_From_>()))>> :
    std::true_type
{
};

template <
    class _From_,
    class _To_>
struct __invoke_nothrow_convertible :
    std::bool_constant<noexcept(__fake_copy_init<_To_>(__returns_exactly<_From_>()))>
{
};

template <
    class   _Result_,
    bool    _NoThrow_>
struct __invoke_common_traits {
    using type = _Result_;
    using __is_invocable = std::true_type;
    using __is_nothrow_invocable = std::bool_constant<_NoThrow_>;

    template <class _Rx>
    using __is_invocable_r = std::bool_constant<std::disjunction_v<std::is_void<_Rx>, __invoke_convertible<type, _Rx>>>;

    template <class _Rx>
    using __is_nothrow_invocable_r = std::bool_constant<std::conjunction_v<__is_nothrow_invocable,
        std::disjunction<std::is_void<_Rx>, std::conjunction<__invoke_convertible<type, _Rx>, __invoke_nothrow_convertible<type, _Rx>>>>>;
};

template <
    class _Void_,
    class _Callable_>
struct __invoke_traits_zero {
    using __is_invocable = std::false_type;
    using __is_nothrow_invocable = std::false_type;

    template <class _Rx>
    using __is_invocable_r = std::false_type;

    template <class _Rx>
    using __is_nothrow_invocable_r = std::false_type;
};

template <class _Callable_>
using __decltype_invoke_zero = decltype(std::declval<_Callable_>()());

template <class _Callable_>
struct __invoke_traits_zero<std::void_t<__decltype_invoke_zero<_Callable_>>, _Callable_> :
    __invoke_common_traits<__decltype_invoke_zero<_Callable_>, noexcept(std::declval<_Callable_>()())>
{
};

template <
    class       _Void_,
    class...    _Args_>
struct __invoke_traits_nonzero {
    using __is_invocable = std::false_type;
    using __is_nothrow_invocable = std::false_type;

    template <class _Rx>
    using __is_invocable_r = std::false_type;

    template <class _Rx>
    using __is_nothrow_invocable_r = std::false_type;
};

template <
    class       _Callable_,
    class       _FirstType_,
    class...    _Args_>
using __decltype_invoke_nonzero = decltype(__invoker_type<_Callable_, _FirstType_>::__call(
    std::declval<_Callable_>(), std::declval<_FirstType_>(), std::declval<_Args_>()...));

template <
    class       _Callable_,
    class       _FirstType_,
    class...    _Args_>
struct __invoke_traits_nonzero<std::void_t<__decltype_invoke_nonzero<_Callable_, _FirstType_, _Args_...>>, _Callable_, _FirstType_, _Args_...> :
    __invoke_common_traits<__decltype_invoke_nonzero<_Callable_, _FirstType_, _Args_...>, noexcept(__invoker_type<_Callable_, _FirstType_>::__call(
        std::declval<_Callable_>(), std::declval<_FirstType_>(), std::declval<_Args_>()...))>
{
};

template <
    class       _Callable_,
    class...    _Args_>
using __select_invoke_traits = std::conditional_t<sizeof...(_Args_) == 0, __invoke_traits_zero<void, _Callable_>,
    __invoke_traits_nonzero<void, _Callable_, _Args_...>>;

template <
    class       _Callable_,
    class ...   _Args_>
constexpr inline bool is_invocable_v = __select_invoke_traits<_Callable_, _Args_...>::__is_invocable::value;

template <
    class       _Callable_,
    class...    _Args_>
inline constexpr bool is_nothrow_invocable_v = __select_invoke_traits<_Callable_, _Args_...>::__is_nothrow_invocable::value;

template <
    class       _Callable_,
    class...    _Args_>
inline constexpr bool is_invocable_r = __select_invoke_traits<_Callable_, _Args_...>::__is_invocable_r::value;

template <
    class       _Callable_,
    class...    _Args_>
inline constexpr bool is_nothrow_invocable_r = __select_invoke_traits<_Callable_, _Args_...>::__is_nothrow_invocable_r::value;

template <
    class       _Callable_,
    class...    _Args_>
using invoke_result_type = typename __select_invoke_traits<_Callable_, _Args_...>::type;

template <class _Callable_>
constexpr auto invoke(_Callable_&& __callable) noexcept(noexcept(std::forward<_Callable_>(__callable)()))
    -> decltype(std::forward<_Callable_>(__callable)())
{
    static_assert(is_invocable_v<_Callable_>, "invoke argument is not callable");
    return std::forward<_Callable_>(__callable)();
}

#define __INVOKER_CALL __invoker_type<_Callable_, _FirstArgument_>::__call( \
    std::forward<_Callable_>(__callable), \
    std::forward<_FirstArgument_>(__first_argument), \
    std::forward<_Args_>(__args)...) 

template <
    class       _Callable_,
    class       _FirstArgument_,
    class...    _Args_>
constexpr auto invoke(
    _Callable_&&        __callable,
    _FirstArgument_&&   __first_argument,
    _Args_&&...         __args)
    noexcept(noexcept(__invoker_type<_Callable_, _FirstArgument_>::__call(
        std::forward<_Callable_>(__callable), 
        std::forward<_FirstArgument_>(__first_argument), 
        std::forward<_Args_>(__args)...))) 
    -> decltype(__invoker_type<_Callable_, _FirstArgument_>::__call(
        std::forward<_Callable_>(__callable), 
        std::forward<_FirstArgument_>(__first_argument), 
        std::forward<_Args_>(__args)...))
{
    static_assert(is_invocable_v<_Callable_, _FirstArgument_, _Args_...>, "invoke argument is not callable");
    return __invoker_type<_Callable_, _FirstArgument_>::__call(
        std::forward<_Callable_>(__callable),
        std::forward<_FirstArgument_>(__first_argument),
        std::forward<_Args_>(__args)...);
}

#undef __INVOKER_CALL

__RAZE_TYPE_TRAITS_NAMESPACE_END