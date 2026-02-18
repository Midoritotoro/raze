#pragma once 

#include <numeric>

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/CallingConventions.h>

#include <raze/RazeNamespace.h>
#include <raze/compatibility/SimdCompatibility.h>

#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
#  include <cpuid.h>
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)


__RAZE_ARCH_NAMESPACE_BEGIN

void cpuid(
	uint32 regs[4],
	uint32 leaf) noexcept
{
#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
	__get_cpuid(leaf, regs, regs + 1, regs + 2, regs + 3);
#else
	__cpuid((int*)regs, leaf);
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
}

void cpuidex(
    uint32 regs[4],
    uint32 leaf, 
    uint32 subleaf) noexcept
{
#if defined(raze_cpp_msvc) || \
    (defined(raze_cpp_clang) && raze_cpp_clang >= 1810) || \
    (defined(raze_cpp_gnu) && raze_cpp_gnu >= 1100)
    __cpuidex((int*)regs, leaf, subleaf);
#else
    uint32* eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf)
    );
#endif
}

__RAZE_ARCH_NAMESPACE_END
