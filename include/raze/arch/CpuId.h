#pragma once 

#include <numeric>

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/CallingConventions.h>

#include <raze/compatibility/Compatibility.h>


#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
#  include <cpuid.h>
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)


__RAZE_ARCH_NAMESPACE_BEGIN

raze_always_inline void cpuid(
	uint32 __regs[4],
	uint32 __leaf) noexcept
{
#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
	__get_cpuid(__leaf, __regs, __regs + 1, __regs + 2, __regs + 3);
#else
	__cpuid(reinterpret_cast<int*>(__regs), __leaf);
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
}

raze_always_inline void cpuidex(
    uint32 __regs[4],
    uint32 __leaf,
    uint32 __subleaf) noexcept
{
#if defined(raze_cpp_msvc) || \
    (defined(raze_cpp_clang) && raze_cpp_clang >= 1810) || \
    (defined(raze_cpp_gnu) && raze_cpp_gnu >= 1100)
    __cpuidex(reinterpret_cast<int*>(__regs), __leaf, __subleaf);
#else
    uint32* __eax = &__regs[0], *__ebx = &__regs[1], *__ecx = &__regs[2], *__edx = &__regs[3];
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*__eax), "=b"(*__ebx), "=c"(*__ecx), "=d"(*__edx)
        : "a"(__leaf), "c"(__subleaf)
    );
#endif
}

__RAZE_ARCH_NAMESPACE_END
