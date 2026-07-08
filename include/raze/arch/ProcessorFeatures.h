#pragma once

#include <bitset>
#include <vector>

#include <array>

#include <raze/Types.h>
#include <raze/arch/CpuId.h>

#include <raze/compatibility/Nodiscard.h>
#include <raze/compatibility/Inline.h>

#include <iostream>
#include <src/raze/math/BitTest.h>


__RAZE_ARCH_NAMESPACE_BEGIN

enum class __features : u8 {
    ERMS, SSE, SSE2, SSE3, SSSE3, SSE41, SSE42, AVX, AVX2, FMA3,
    AVX512F, AVX512BW, AVX512PF, AVX512ER, AVX512CD, AVX512VL, 
    AVX512DQ, AVX512VBMI, AVX512VBMI2, POPCNT
};

class ProcessorFeatures
{
public:
    raze_nodiscard raze_always_inline static i32 all() noexcept;

    raze_nodiscard raze_always_inline static bool SSE()         noexcept;
    raze_nodiscard raze_always_inline static bool SSE2()        noexcept;
    raze_nodiscard raze_always_inline static bool SSE3()        noexcept;
    raze_nodiscard raze_always_inline static bool SSSE3()       noexcept;
    raze_nodiscard raze_always_inline static bool SSE41()       noexcept;
    raze_nodiscard raze_always_inline static bool SSE42()       noexcept;
    raze_nodiscard raze_always_inline static bool AVX()         noexcept;
    raze_nodiscard raze_always_inline static bool AVX2()        noexcept;
    raze_nodiscard raze_always_inline static bool FMA3()        noexcept;
    raze_nodiscard raze_always_inline static bool AVX512F()     noexcept;
    raze_nodiscard raze_always_inline static bool AVX512BW()    noexcept;
    raze_nodiscard raze_always_inline static bool AVX512PF()    noexcept;
    raze_nodiscard raze_always_inline static bool AVX512ER()    noexcept;
    raze_nodiscard raze_always_inline static bool AVX512CD()    noexcept;
    raze_nodiscard raze_always_inline static bool AVX512VL()    noexcept;
    raze_nodiscard raze_always_inline static bool AVX512DQ()    noexcept;
    raze_nodiscard raze_always_inline static bool AVX512VBMI()  noexcept;
    raze_nodiscard raze_always_inline static bool AVX512VBMI2() noexcept;
    raze_nodiscard raze_always_inline static bool ERMS() noexcept;

    raze_nodiscard raze_always_inline static bool POPCNT()      noexcept;

    template <arch::ISA _Feature_> 
    raze_nodiscard raze_always_inline static bool isSupported() noexcept;

    template <__features _Feature_>
    static raze_always_inline bool has(i32 __all) noexcept {
        constexpr auto __bit = 1u << static_cast<u32>(_Feature_);
        return (__all & __bit) != 0;
    }
private:
    class ProcessorFeaturesInternal
    {
        raze_nodiscard raze_always_inline static i32 highest_function_id(u32* registers) noexcept;
    public:
        raze_always_inline ProcessorFeaturesInternal() noexcept;

        bool _sse = false;
        bool _sse2 = false;
        bool _sse3 = false;
        bool _sse41 = false;
        bool _sse42 = false;
        bool _ssse3 = false;
        
        bool _avx = false;
        bool _avx2 = false;
        bool _fma3 = false;

        bool _avx512f = false;
        bool _avx512bw = false;
        bool _avx512pf = false;
        bool _avx512er = false;
        bool _avx512cd = false;
        bool _avx512vl = false;
        bool _avx512dq = false;
        bool _avx512vbmi = false;
        bool _avx512vbmi2 = false;

        bool _popcnt = false;
        bool _erms = false;

        i32 _all = 0;
    };

    static inline ProcessorFeaturesInternal _processorFeaturesInternal;
};

raze_always_inline i32 ProcessorFeatures::ProcessorFeaturesInternal::highest_function_id(u32* registers) noexcept {
    cpuid(registers, 0);
    return registers[0];
}

ProcessorFeatures::ProcessorFeaturesInternal::ProcessorFeaturesInternal() noexcept {
    std::array<u32, 4> registers;

    const auto leafCount = highest_function_id(registers.data());
 
    if (leafCount >= 1) {
        std::memset(registers.data(), 0, registers.size() * sizeof(u32));
        cpuidex(registers.data(), 1, 0);

        const auto leaf1Ecx = registers[2];
        
#if defined(raze_processor_x86_64)
        _sse    = true;
        _sse2   = true;
#else
        const auto leaf1Edx = registers[3];

        _sse    = (leaf1Edx >> 25) & 1;
        _sse2   = (leaf1Edx >> 26) & 1;
#endif

        _sse3   = (leaf1Ecx & 1);
        _ssse3  = (leaf1Ecx >> 9) & 1;
        _sse41  = (leaf1Ecx >> 19) & 1;
        _sse42  = (leaf1Ecx >> 20) & 1;
        
        _popcnt = (leaf1Ecx >> 23) & 1;
        _avx    = (leaf1Ecx >> 28) & 1;
        _fma3   = ((leaf1Ecx >> 27) & 1) && ((leaf1Ecx >> 12) & 1);
    }

    if (leafCount >= 7) {
        std::memset(registers.data(), 0, registers.size() * sizeof(u32));
        cpuidex(registers.data(), 7, 0);

        const auto leaf7Ebx = registers[1];
        const auto leaf7Ecx = registers[2];
        
        _avx2           = (leaf7Ebx >> 5) & 1;

        _avx512f        = (leaf7Ebx >> 16) & 1;
        _avx512dq       = (leaf7Ebx >> 17) & 1;
        _avx512bw       = (leaf7Ebx >> 30) & 1;
        _avx512pf       = (leaf7Ebx >> 26) & 1;
        _avx512er       = (leaf7Ebx >> 27) & 1;
        _avx512cd       = (leaf7Ebx >> 28) & 1;
        _avx512vl       = (leaf7Ebx >> 31) & 1;
        _erms           = (leaf7Ebx >> 9) & 1;

        _avx512vbmi     = (leaf7Ecx >> 1) & 1;
        _avx512vbmi2    = (leaf7Ecx >> 6) & 1;
    }

    _all = (i32(_sse) << static_cast<u32>(__features::SSE)) |
        (i32(_sse2) << static_cast<u32>(__features::SSE2)) |
        (i32(_sse3) << static_cast<u32>(__features::SSE3)) |
        (i32(_ssse3) << static_cast<u32>(__features::SSSE3)) |
        (i32(_sse41) << static_cast<u32>(__features::SSE41)) |
        (i32(_sse42) << static_cast<u32>(__features::SSE42)) |
        (i32(_avx) << static_cast<u32>(__features::AVX)) |
        (i32(_avx2) << static_cast<u32>(__features::AVX2)) |
        (i32(_fma3) << static_cast<u32>(__features::FMA3)) |
        (i32(_avx512f) << static_cast<u32>(__features::AVX512F)) |
        (i32(_avx512bw) << static_cast<u32>(__features::AVX512BW)) |
        (i32(_avx512pf) << static_cast<u32>(__features::AVX512PF)) |
        (i32(_avx512er) << static_cast<u32>(__features::AVX512ER)) |
        (i32(_avx512cd) << static_cast<u32>(__features::AVX512CD)) |
        (i32(_avx512vl) << static_cast<u32>(__features::AVX512VL)) |
        (i32(_avx512dq) << static_cast<u32>(__features::AVX512DQ)) |
        (i32(_avx512vbmi) << static_cast<u32>(__features::AVX512VBMI)) |
        (i32(_avx512vbmi2) << static_cast<u32>(__features::AVX512VBMI2)) |
        (i32(_popcnt) << static_cast<u32>(__features::POPCNT)) |
        (i32(_erms) << static_cast<u32>(__features::ERMS));
}

i32 ProcessorFeatures::all() noexcept {
    return _processorFeaturesInternal._all;
}

bool ProcessorFeatures::ERMS() noexcept {
    return _processorFeaturesInternal._erms;
}

bool ProcessorFeatures::SSE() noexcept {
    return _processorFeaturesInternal._sse;
}

bool ProcessorFeatures::SSE2() noexcept {
    return _processorFeaturesInternal._sse2;
}

bool ProcessorFeatures::SSE3() noexcept {
    return _processorFeaturesInternal._sse3;
}

bool ProcessorFeatures::SSSE3() noexcept {
    return _processorFeaturesInternal._ssse3;
}

bool ProcessorFeatures::SSE41() noexcept {
    return _processorFeaturesInternal._sse41;
}

bool ProcessorFeatures::SSE42() noexcept {
    return _processorFeaturesInternal._sse42;
}

bool ProcessorFeatures::AVX() noexcept {
    return _processorFeaturesInternal._avx;
}

bool ProcessorFeatures::AVX2() noexcept {
    return _processorFeaturesInternal._avx2;
}

bool ProcessorFeatures::AVX512F() noexcept {
    return _processorFeaturesInternal._avx512f;
}

bool ProcessorFeatures::AVX512BW() noexcept {
    return _processorFeaturesInternal._avx512bw;
}

bool ProcessorFeatures::AVX512PF() noexcept {
    return _processorFeaturesInternal._avx512pf;
}

bool ProcessorFeatures::AVX512ER() noexcept {
    return _processorFeaturesInternal._avx512er;
}

bool ProcessorFeatures::AVX512CD() noexcept {
    return _processorFeaturesInternal._avx512cd;
}

bool ProcessorFeatures::AVX512VL() noexcept {
    return _processorFeaturesInternal._avx512vl;
}

bool ProcessorFeatures::AVX512DQ() noexcept {
    return _processorFeaturesInternal._avx512dq;
}

bool ProcessorFeatures::AVX512VBMI() noexcept {
    return _processorFeaturesInternal._avx512vbmi;
}

bool ProcessorFeatures::AVX512VBMI2() noexcept {
    return _processorFeaturesInternal._avx512vbmi2;
}

bool ProcessorFeatures::POPCNT() noexcept {
    return _processorFeaturesInternal._popcnt;
}

template <arch::ISA _Feature_>
bool ProcessorFeatures::isSupported() noexcept {
    if      constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::SSE))
        return _processorFeaturesInternal._sse;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::SSE2))
        return _processorFeaturesInternal._sse2;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::SSE3))
        return _processorFeaturesInternal._sse3;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::SSSE3))
        return _processorFeaturesInternal._ssse3;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::SSE41))
        return _processorFeaturesInternal._sse41;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::SSE42))
        return _processorFeaturesInternal._sse42;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX))
        return _processorFeaturesInternal._avx;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX2))
        return _processorFeaturesInternal._avx2;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::FMA3))
        return _processorFeaturesInternal._fma3;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX))
        return _processorFeaturesInternal._avx;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX2FMA3))
        return _processorFeaturesInternal._avx2 && _processorFeaturesInternal._fma3;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512F))
        return _processorFeaturesInternal._avx512f;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512BW))
        return _processorFeaturesInternal._avx512bw;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512DQ))
        return _processorFeaturesInternal._avx512dq;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512BWDQ))
        return _processorFeaturesInternal._avx512dq && _processorFeaturesInternal._avx512bw;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VLF))
        return _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512f;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VLBW))
        return _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512bw;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VLDQ))
        return _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VLBWDQ))
        return _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq && _processorFeaturesInternal._avx512bw;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMI))
        return _processorFeaturesInternal._avx512vbmi;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMI2))
        return _processorFeaturesInternal._avx512vbmi2;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMIDQ))
        return _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512dq;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMI2DQ))
        return _processorFeaturesInternal._avx512vbmi2 && _processorFeaturesInternal._avx512dq;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMIVL))
        return _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vl;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMI2VL))
        return _processorFeaturesInternal._avx512vbmi2 && _processorFeaturesInternal._avx512vl;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMIVLDQ))
        return _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq;
    else if constexpr (static_cast<i8>(_Feature_) == static_cast<i8>(ISA::AVX512VBMI2VLDQ))
        return _processorFeaturesInternal._avx512vbmi2 && _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq;
}

__RAZE_ARCH_NAMESPACE_END
