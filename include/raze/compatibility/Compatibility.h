#pragma once 

// Clang attributes
// https://clang.llvm.org/docs/AttributeReference.html#always-inline-force-inline
// Clang builtins
// https://clang.llvm.org/docs/LanguageExtensions.html

// Msvc attributes
// https://learn.microsoft.com/en-us/cpp/cpp/declspec?view=msvc-170
// Msvc SAL
// https://learn.microsoft.com/en-us/cpp/code-quality/using-sal-annotations-to-reduce-c-cpp-code-defects?view=msvc-170

// Gcc attributes
// https://ohse.de/uwe/articles/gcc-attributes.html and https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/AlignmentMacros.h>

#include <raze/compatibility/BranchPrediction.h>
#include <raze/compatibility/CallingConventions.h>

#include <raze/compatibility/Inline.h>
#include <raze/compatibility/CxxVersionDetection.h>

#include <raze/compatibility/FunctionAttributes.h>
#include <raze/compatibility/LanguageFeatures.h>

#include <raze/compatibility/MemoryMacros.h>
#include <raze/compatibility/Nodiscard.h>

#include <raze/compatibility/SimdCompatibility.h>
#include <raze/compatibility/SystemDetection.h>

#include <raze/compatibility/UnreachableCode.h>
#include <raze/compatibility/Warnings.h>

#include <raze/compatibility/StaticOperators.h>
#include <cstddef>

raze_disable_warning_msvc(4067)


#if !defined(__simd_inline_constexpr)
#  define __simd_inline_constexpr raze_constexpr_cxx20 raze_always_inline
#endif // !defined(__simd_inline_constexpr)

#if !defined(__simd_nodiscard_inline_constexpr)
#  define __simd_nodiscard_inline_constexpr raze_nodiscard raze_always_inline raze_constexpr_cxx20
#endif // !defined(__simd_nodiscard_inline_constexpr)

#if !defined(__simd_nodiscard_inline)
#  define __simd_nodiscard_inline raze_nodiscard raze_always_inline
#endif // !defined(__simd_nodiscard_inline)
