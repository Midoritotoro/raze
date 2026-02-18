#pragma once 

#if !defined(RAZE_ECHO)
#  define RAZE_ECHO(...) __VA_ARGS__
#endif

#if defined(_MSC_VER)
#  define raze_cpp_msvc			(_MSC_VER)
#  define raze_cpp_msvc_only	raze_cpp_msvc

#  if defined(__clang__)
#    undef raze_cpp_msvc_only
#    define raze_cpp_clang ((__clang_major__ * 100) + __clang_minor__)
#    define raze_cpp_clang_only raze_cpp_clang
#  endif // defined(__clang__)
#

#elif defined(__GNUC__)
#  define raze_cpp_gnu          (__GNUC__ * 100 + __GNUC_MINOR__)

#  if defined(__MINGW32__)
#    define raze_cpp_mingw
#  endif

#  if defined(__clang__)
#    if defined(__apple_build_version__)

#      if __apple_build_version__   >= 14030022 // Xcode 14.3
#        define raze_cpp_clang 1500

#      elif __apple_build_version__ >= 14000029 // Xcode 14.0
#        define raze_cpp_clang 1400

#      elif __apple_build_version__ >= 13160021 // Xcode 13.3
#        define raze_cpp_clang 1300

#      elif __apple_build_version__ >= 13000029 // Xcode 13.0
#        define raze_cpp_clang 1200

#      elif __apple_build_version__ >= 12050022 // Xcode 12.5
#        define raze_cpp_clang 1110

#      elif __apple_build_version__ >= 12000032 // Xcode 12.0
#        define raze_cpp_clang 1000

#      elif __apple_build_version__ >= 11030032 // Xcode 11.4
#        define raze_cpp_clang 900

#      elif __apple_build_version__ >= 11000033 // Xcode 11.0
#        define raze_cpp_clang 800

#      else
#        error ""
#      endif // __apple_build_version__

#    else
#      define raze_cpp_clang ((__clang_major__ * 100) + __clang_minor__)
#    endif // defined(__apple_build_version__)

#    define raze_cpp_clang_only raze_cpp_clang


#    if !defined(__has_extension)
#      define __has_extension __has_feature
#    endif // !defined(__has_extension)
#  else

#    define raze_cpp_gnu_only raze_cpp_gnu
#  endif // defined(__clang__)

#endif // defined(_MSC_VER) || defined(__GNUC__)

#define __RAZE_REPEAT__(X) X

#define __RAZE_REPEAT_1(X)  __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_2(X)  __RAZE_REPEAT_1(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_3(X)  __RAZE_REPEAT_2(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_4(X)  __RAZE_REPEAT_3(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_5(X)  __RAZE_REPEAT_4(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_6(X)  __RAZE_REPEAT_5(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_7(X)  __RAZE_REPEAT_6(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_8(X)  __RAZE_REPEAT_7(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_9(X)  __RAZE_REPEAT_8(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_10(X) __RAZE_REPEAT_9(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_11(X) __RAZE_REPEAT_10(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_12(X) __RAZE_REPEAT_11(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_13(X) __RAZE_REPEAT_12(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_14(X) __RAZE_REPEAT_13(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_15(X) __RAZE_REPEAT_14(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_16(X) __RAZE_REPEAT_15(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_17(X) __RAZE_REPEAT_16(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_18(X) __RAZE_REPEAT_17(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_19(X) __RAZE_REPEAT_18(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_20(X) __RAZE_REPEAT_19(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_21(X) __RAZE_REPEAT_20(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_22(X) __RAZE_REPEAT_21(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_23(X) __RAZE_REPEAT_22(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_24(X) __RAZE_REPEAT_23(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_25(X) __RAZE_REPEAT_24(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_26(X) __RAZE_REPEAT_25(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_27(X) __RAZE_REPEAT_26(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_28(X) __RAZE_REPEAT_27(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_29(X) __RAZE_REPEAT_28(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_30(X) __RAZE_REPEAT_29(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_31(X) __RAZE_REPEAT_30(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_32(X) __RAZE_REPEAT_31(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_33(X) __RAZE_REPEAT_32(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_34(X) __RAZE_REPEAT_33(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_35(X) __RAZE_REPEAT_34(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_36(X) __RAZE_REPEAT_35(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_37(X) __RAZE_REPEAT_36(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_38(X) __RAZE_REPEAT_37(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_39(X) __RAZE_REPEAT_38(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_40(X) __RAZE_REPEAT_39(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_41(X) __RAZE_REPEAT_40(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_42(X) __RAZE_REPEAT_41(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_43(X) __RAZE_REPEAT_42(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_44(X) __RAZE_REPEAT_43(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_45(X) __RAZE_REPEAT_44(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_46(X) __RAZE_REPEAT_45(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_47(X) __RAZE_REPEAT_46(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_48(X) __RAZE_REPEAT_47(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_49(X) __RAZE_REPEAT_48(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_50(X) __RAZE_REPEAT_49(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_51(X) __RAZE_REPEAT_50(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_52(X) __RAZE_REPEAT_51(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_53(X) __RAZE_REPEAT_52(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_54(X) __RAZE_REPEAT_53(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_55(X) __RAZE_REPEAT_54(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_56(X) __RAZE_REPEAT_55(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_57(X) __RAZE_REPEAT_56(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_58(X) __RAZE_REPEAT_57(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_59(X) __RAZE_REPEAT_58(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_60(X) __RAZE_REPEAT_59(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_61(X) __RAZE_REPEAT_60(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_62(X) __RAZE_REPEAT_61(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_63(X) __RAZE_REPEAT_62(X), __RAZE_REPEAT__(X)
#define __RAZE_REPEAT_64(X) __RAZE_REPEAT_63(X), __RAZE_REPEAT__(X)

#define __RAZE_PP_CAT(a,b) a##b
#define RAZE_PP_CAT(a,b) __RAZE_PP_CAT(a,b)

#define __RAZE_REPEAT_N(N, X) RAZE_PP_CAT(__RAZE_REPEAT_, N)(X)


#define __RAZE_DUPLICATE__(X) X

#define __RAZE_DUPLICATE_1(X)  __RAZE_DUPLICATE__(X)
#define __RAZE_DUPLICATE_2(X)  __RAZE_DUPLICATE_1(X) __RAZE_DUPLICATE__(X)
#define __RAZE_DUPLICATE_3(X)  __RAZE_DUPLICATE_2(X) __RAZE_DUPLICATE__(X)
#define __RAZE_DUPLICATE_4(X)  __RAZE_DUPLICATE_3(X) __RAZE_DUPLICATE__(X)
#define __RAZE_DUPLICATE_5(X)  __RAZE_DUPLICATE_4(X) __RAZE_DUPLICATE__(X)
#define __RAZE_DUPLICATE_6(X)  __RAZE_DUPLICATE_5(X) __RAZE_DUPLICATE__(X)
#define __RAZE_DUPLICATE_7(X)  __RAZE_DUPLICATE_6(X) __RAZE_DUPLICATE__(X)
#define __RAZE_DUPLICATE_8(X)  __RAZE_DUPLICATE_7(X) __RAZE_DUPLICATE__(X)

#define __RAZE_DUPLICATE_N(N, X) RAZE_PP_CAT(__RAZE_DUPLICATE_, N)(X)
