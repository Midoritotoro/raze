#pragma once 

#include <numeric>

#include <raze/compatibility/CompilerDetection.h>
#include <raze/compatibility/CallingConventions.h>

#include <raze/compatibility/Compatibility.h>


#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
#  include <cpuid.h>
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)


__RAZE_ARCH_NAMESPACE_BEGIN

raze_always_inline void cpuid(u32 regs[4], u32 leaf) noexcept {
#if (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
	__get_cpuid(leaf, regs, regs + 1, regs + 2, regs + 3);
#else
	__cpuid(reinterpret_cast<int*>(regs), leaf);
#endif // (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_msvc)
}

raze_always_inline void cpuidex(u32 regs[4], u32 leaf, u32 subleaf) noexcept {
#if defined(raze_cpp_msvc) || \
    (defined(raze_cpp_clang) && raze_cpp_clang >= 1810) || \
    (defined(raze_cpp_gnu) && raze_cpp_gnu >= 1100)
    __cpuidex(reinterpret_cast<int*>(regs), leaf, subleaf);
#else
    u32* eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf)
    );
#endif
}

__RAZE_ARCH_NAMESPACE_END
