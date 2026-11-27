#pragma once

#include <raze/Types.h>
#include <src/raze/RazeNamespace.h>
#include <src/raze/arch/ProcessorDetection.h>
#include <src/raze/compatibility/CompilerDetection.h>
#include <src/raze/compatibility/Inline.h>

#if defined(raze_processor_x86)
#  if defined(raze_cpp_msvc)
#    include <intrin.h>
#  elif defined(raze_cpp_clang) || defined(raze_cpp_gnu)
#    include <cpuid.h>
#  endif
#endif // defined(raze_processor_x86)

__RAZE_ARCH_NAMESPACE_BEGIN

raze_always_inline void cpuid(u32 regs[4], u32 leaf) noexcept {
#if defined(raze_processor_x86)
#  if defined(raze_cpp_msvc)
    __cpuid(reinterpret_cast<int*>(regs), static_cast<int>(leaf));
#  elif defined(raze_cpp_clang) || defined(raze_cpp_gnu)
    __cpuid(leaf, regs[0], regs[1], regs[2], regs[3]);
#  else
    __asm__ __volatile__(
        "cpuid"
        : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
        : "a"(leaf), "c"(0)
    );
#  endif
#else
    regs[0] = regs[1] = regs[2] = regs[3] = 0;
#endif
}

raze_always_inline void cpuidex(u32 regs[4], u32 leaf, u32 subleaf) noexcept {
#if defined(raze_processor_x86)
#  if defined(raze_cpp_msvc)
    __cpuidex(reinterpret_cast<int*>(regs), static_cast<int>(leaf), static_cast<int>(subleaf));
#  elif (defined(raze_cpp_clang) || defined(raze_cpp_gnu)) && !defined(raze_cpp_nvhpc)
    __cpuid_count(leaf, subleaf, regs[0], regs[1], regs[2], regs[3]);
#  else
    u32* eax = &regs[0], *ebx = &regs[1], *ecx = &regs[2], *edx = &regs[3];
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf)
    );
#  endif
#else
    regs[0] = regs[1] = regs[2] = regs[3] = 0;
#endif
}

raze_always_inline u64 xgetbv(u32 xcr) noexcept {
#if defined(raze_processor_x86)
#  if defined(raze_cpp_msvc)
    return static_cast<u64>(_xgetbv(xcr));
#  else
    u32 eax = 0;
    u32 edx = 0;
    __asm__ __volatile__(
        "xgetbv"
        : "=a"(eax), "=d"(edx)
        : "c"(xcr)
    );
    return (static_cast<u64>(edx) << 32) | static_cast<u64>(eax);
#  endif
#else
    (void)xcr;
    return 0;
#endif
}

__RAZE_ARCH_NAMESPACE_END