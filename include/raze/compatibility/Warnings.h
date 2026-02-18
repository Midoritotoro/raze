#pragma once 

#include <raze/compatibility/Nodiscard.h>

// Warnings

#define raze_do_pragma(text)                      _Pragma(#text)

#if defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#  undef raze_do_pragma
#endif // defined(raze_cpp_msvc) && !defined(raze_cpp_clang)

#if !defined(raze_warning_push)
#  if defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#    define raze_warning_push __pragma(warning(push))
#  elif defined(raze_cpp_clang)
#    define raze_warning_push raze_do_pragma(clang diagnostic push)
#  elif defined(raze_cpp_gnu) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#    define raze_warning_push raze_do_pragma(GCC diagnostic push)
#  else 
#    define raze_warning_push
#  endif // (defined(raze_cpp_msvc) && !defined(raze_cpp_clang)) || defined(raze_cpp_clang)
         // || defined(raze_cpp_gnu) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#endif // !defined(raze_warning_push)


#if !defined(raze_warning_pop)
#  if defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#    define raze_warning_pop __pragma(warning(pop))
#  elif defined(raze_cpp_clang)
#    define raze_warning_pop raze_do_pragma(clang diagnostic pop)
#  elif defined(raze_cpp_gnu) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#    define raze_warning_pop raze_do_pragma(GCC diagnostic pop)
#  else
#    define raze_warning_pop
#  endif // (defined(raze_cpp_msvc) && !defined(raze_cpp_clang)) || defined(raze_cpp_clang)
         // || defined(raze_cpp_gnu) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#endif // !defined(raze_warning_pop)


#if !defined(raze_disable_warning_msvc)
#  if defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#    define raze_disable_warning_msvc(number) __pragma(warning(disable: number))
#  else
#    define raze_disable_warning_msvc(number)
#  endif // defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#endif // !defined(raze_disable_warning_msvc)


#if !defined(raze_disable_warning_clang)
#  if defined(raze_cpp_clang)
#    define raze_disable_warning_clang(text) raze_do_pragma(clang diagnostic ignored text)
#  else
#    define raze_disable_warning_clang(text)
#  endif // defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#endif // !defined(raze_disable_warning_clang)


#if !defined(raze_disable_warning_gcc)
#  if defined(raze_cpp_gnu) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#    define raze_disable_warning_gcc(text) raze_do_pragma(GCC diagnostic ignored text)
#  else
#    define raze_disable_warning_gcc(text) 
#  endif // defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#endif // !defined(raze_disable_warning_gcc)


#if !defined(raze_disable_warning_deprecated)
#  if defined(raze_cpp_msvc) && !defined(raze_cpp_clang)
#    define raze_disable_warning_deprecated raze_disable_warning_msvc(4996)
#  elif defined(raze_cpp_clang)
#    define raze_disable_warning_deprecated raze_disable_warning_clang("-Wdeprecated-declarations")
#  elif defined(raze_cpp_gnu) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#    define raze_disable_warning_deprecated raze_warning_disable_gcc("-Wdeprecated-declarations")
#  else
#    define raze_disable_warning_deprecated
#  endif // (defined(raze_cpp_msvc) && !defined(raze_cpp_clang)) || defined(raze_cpp_clang)
         // || defined(raze_cpp_gnu) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#endif // !defined(raze_disable_warning_deprecated)


#if !defined(raze_cpp_warnings)
#  define raze_no_warnings
#endif // !defined(raze_cpp_warnings)


#if defined(raze_no_warnings)
#  if defined(raze_cpp_msvc)
     raze_disable_warning_msvc(4828) /* ���� �������� ����, ������������ �� �������� 0xX, ������� �������� ������������ � ������� �������� ���������. */
     raze_disable_warning_msvc(4251) /* ����� 'type' ������ ����� dll-���������, ������� ����� �������������� ��������� ������ 'type2' */
     raze_disable_warning_msvc(4244) /* �������������� �� 'type1' � 'type2', ��������� ������ ������ */
     raze_disable_warning_msvc(4275) /* ������������� ����� ������, ��������� �� DLL-����������, ������������ � �������� �������� ��� �������������� ����� ������ DLL-���������� */
     raze_disable_warning_msvc(4514) /* ������� ���������� �������, �� ������� ��� ������ */
     raze_disable_warning_msvc(4800) /* 'type' : �������������� ���������� �������� bool 'true' ��� 'false' (�������������� � ������������������) */
     raze_disable_warning_msvc(4097) /* typedef-��� 'identifier1' ������������ ��� ������� ����� ������ 'identifier2' */
     raze_disable_warning_msvc(4706) /* ���������� � �������� ��������� */
     raze_disable_warning_msvc(4355) /* 'this' : ������������ � ������ ��������������� ������� ��������� */
     raze_disable_warning_msvc(4710) /* ������� �� �������� */
     raze_disable_warning_msvc(4530) /* ������������ ���������� ���������� C++, �� ��������� �������� �� ��������. ������� /EHsc */
     raze_disable_warning_msvc(4006)
     raze_disable_warning_msvc(4715)
     raze_disable_warning_msvc(4267)
#  endif // defined(raze_cpp_msvc)

#endif // defined(raze_no_warnings)

#if !defined(raze_nodiscard_return_raw_ptr)
#  define raze_nodiscard_return_raw_ptr \
        raze_nodiscard_with_warning("This function allocates memory and returns a raw pointer. " \
            "Discarding the return value will cause a memory leak.")
#endif // !defined(raze_nodiscard_return_raw_ptr)


#if !defined(raze_nodiscard_thread_constructor)
#  define raze_nodiscard_thread_constructor \
    raze_nodiscard_constructor_with_warning("Creating a thread object without assigning it to a variable " \
        "may lead to unexpected behavior and resource leaks. Ensure " \
        "the thread is properly managed.")
#endif // !defined(raze_nodiscard_thread_constructor)


#if !defined(raze_nodiscard_remove_algorithm)
#  define raze_nodiscard_remove_algorithm \
        raze_nodiscard_with_warning("The 'remove' and 'remove_if' algorithms return the iterator past the last element " \
            "that should be kept. You need to call container.erase(result, container.end()) afterwards. " \
            "In C++20, 'std::erase' and 'std::erase_if' are simpler replacements for these two steps.")
#endif // !defined(raze_nodiscard_remove_algorithm)


#if !defined(raze_deprecated_warning)
#  if defined(raze_cpp_msvc)
#    define raze_deprecated_warning(message)                                           \
       __pragma(warning(push))                                                         \
       __pragma(warning(disable: 4996))                                                \
       __pragma(message (__FILE__ "(" __LINE__ ") : warning C4996: " message))         \
       __pragma(warning(pop))
#  elif defined(raze_cpp_clang)
#    define raze_deprecated_warning(message)                                           \
       raze_do_pragma("clang diagnostic push")                                         \
       raze_do_pragma("clang diagnostic warning \"-Wdeprecated-declarations\"")        \
       raze_do_pragma("clang diagnostic ignored \"-Wunused-but-set-variable\"")        \
       raze_do_pragma("message \"" __FILE__ "(" __LINE__ ") : warning: " message "\"") \
       raze_do_pragma("clang diagnostic pop")
#  elif defined(raze_cpp_gnu)
#    define raze_deprecated_warning(message)                                           \
       raze_do_pragma("GCC diagnostic push")                                           \
       raze_do_pragma("GCC diagnostic warning \"-Wdeprecated-declarations\"")          \
       raze_do_pragma("message \"" __FILE__ "(" __LINE__ ") : warning: " message "\"") \ 
         raze_do_pragma("GCC diagnostic pop")
#  else
#    define raze_deprecated_warning(message)
#  endif // defined(raze_cpp_msvc) || defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#endif // !defined(raze_deprecated_warning)