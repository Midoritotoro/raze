#pragma once

#include <array>
#include <cstring>

#include <raze/Types.h>
#include <src/raze/RazeNamespace.h>
#include <src/raze/arch/CpuFeature.h>
#include <src/raze/arch/CpuId.h>
#include <src/raze/arch/ProcessorDetection.h>
#include <src/raze/compatibility/Inline.h>
#include <src/raze/compatibility/Nodiscard.h>

__RAZE_ARCH_NAMESPACE_BEGIN

enum class features : u8 {
    ERMS, SSE, SSE2, SSE3, SSSE3, SSE41, SSE42, AVX, AVX2, FMA3,
    AVX512F, AVX512BW, AVX512PF, AVX512ER, AVX512CD, AVX512VL, 
	AVX512DQ, AVX512VBMI, AVX512VBMI2, POPCNT, UNKNOWN
};

consteval u32 feature_mask_of(ISA isa) noexcept {
    constexpr auto bit = [](features f) constexpr noexcept -> u32 {
        return 1u << static_cast<u32>(f);
    };

    switch (isa) {
        case ISA::SSE:            return bit(features::SSE);
        case ISA::SSE2:           return bit(features::SSE2);
        case ISA::SSE3:           return bit(features::SSE3);
        case ISA::SSSE3:          return bit(features::SSSE3);
        case ISA::SSE41:          return bit(features::SSE41);
        case ISA::SSE42:          return bit(features::SSE42);
        case ISA::AVX:            return bit(features::AVX);
        case ISA::FMA3:           return bit(features::FMA3);
        case ISA::AVX2:           return bit(features::AVX2);
        case ISA::AVX2FMA3:       return bit(features::AVX2) | bit(features::FMA3);
        case ISA::AVX512F:        return bit(features::AVX512F);
        case ISA::AVX512BW:       return bit(features::AVX512F) | bit(features::AVX512BW);
        case ISA::AVX512DQ:       return bit(features::AVX512F) | bit(features::AVX512DQ);
        case ISA::AVX512BWDQ:     return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512DQ);
        case ISA::AVX512VLF:      return bit(features::AVX512F) | bit(features::AVX512VL);
        case ISA::AVX512VLBW:     return bit(features::AVX512F) | bit(features::AVX512VL) | bit(features::AVX512BW);
        case ISA::AVX512VLDQ:     return bit(features::AVX512F) | bit(features::AVX512VL) | bit(features::AVX512DQ);
        case ISA::AVX512VLBWDQ:   return bit(features::AVX512F) | bit(features::AVX512VL) | bit(features::AVX512BW) | bit(features::AVX512DQ);
        case ISA::AVX512VBMI:     return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512VBMI);
        case ISA::AVX512VBMI2:    return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512VBMI) | bit(features::AVX512VBMI2);
        case ISA::AVX512VBMIDQ:   return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512DQ) | bit(features::AVX512VBMI);
        case ISA::AVX512VBMI2DQ:  return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512DQ) | bit(features::AVX512VBMI) | bit(features::AVX512VBMI2);
        case ISA::AVX512VBMIVL:   return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512VL) | bit(features::AVX512VBMI);
        case ISA::AVX512VBMI2VL:  return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512VL) | bit(features::AVX512VBMI) | bit(features::AVX512VBMI2);
        case ISA::AVX512VBMIVLDQ: return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512VL) | bit(features::AVX512DQ) | bit(features::AVX512VBMI);
        case ISA::AVX512VBMI2VLDQ:return bit(features::AVX512F) | bit(features::AVX512BW) | bit(features::AVX512VL) | bit(features::AVX512DQ) | bit(features::AVX512VBMI) | bit(features::AVX512VBMI2);
        default:                  return bit(features::UNKNOWN);
    }
}

consteval features feature_of(ISA isa) noexcept {
	switch (isa) {
	    case ISA::SSE: return features::SSE;
	    case ISA::SSE2: return features::SSE2;
	    case ISA::SSE3: return features::SSE3;
	    case ISA::SSSE3: return features::SSSE3;
	    case ISA::SSE41: return features::SSE41;
	    case ISA::SSE42: return features::SSE42;
	    case ISA::AVX: return features::AVX;
	    case ISA::FMA3: return features::FMA3;
	    case ISA::AVX2: return features::AVX2;
	    case ISA::AVX512F: return features::AVX512F;
	    case ISA::AVX512BW: return features::AVX512BW;
	    case ISA::AVX512VLF: return features::AVX512VL;
	    case ISA::AVX512DQ: return features::AVX512DQ;
	    case ISA::AVX512VBMI: return features::AVX512VBMI;
	    case ISA::AVX512VBMI2: return features::AVX512VBMI2;
	    default: return features::UNKNOWN;
	}
}

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

    template <arch::ISA Feature> 
    raze_nodiscard raze_always_inline static bool isSupported() noexcept;

    template <features Feature>
    static raze_always_inline bool has(i32 all) noexcept {
        constexpr auto bit = 1u << static_cast<u32>(Feature);
        return (all & bit) != 0;
    }

    template <arch::ISA Feature>
    static raze_always_inline bool has_isa(i32 all) noexcept {
        constexpr u32 mask = feature_mask_of(Feature);
        return (static_cast<u32>(all) & mask) == mask;
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
    std::array<u32, 4> registers{};

    const auto leaf_count = highest_function_id(registers.data());

    bool avx_os_support = false;
    bool avx512_os_support = false;

    if (leaf_count >= 1) {
        std::memset(registers.data(), 0, registers.size() * sizeof(u32));
        cpuidex(registers.data(), 1, 0);

        const auto leaf1_ecx = registers[2];
        
#if defined(raze_processor_x86_64)
        _sse    = true;
        _sse2   = true;
#else
        const auto leaf1_edx = registers[3];

        _sse    = (leaf1_edx >> 25) & 1;
        _sse2   = (leaf1_edx >> 26) & 1;
#endif

        _sse3   = (leaf1_ecx & 1) != 0;
        _ssse3  = ((leaf1_ecx >> 9) & 1) != 0;
        _sse41  = ((leaf1_ecx >> 19) & 1) != 0;
        _sse42  = ((leaf1_ecx >> 20) & 1) != 0;
        
        _popcnt = ((leaf1_ecx >> 23) & 1) != 0;

        const bool osxsave = ((leaf1_ecx >> 27) & 1) != 0;
        const u64 xcr0 = osxsave ? xgetbv(0) : 0;

        avx_os_support    = osxsave && ((xcr0 & 0x06ull) == 0x06ull);
        avx512_os_support = osxsave && ((xcr0 & 0xE6ull) == 0xE6ull);

        _avx    = avx_os_support && (((leaf1_ecx >> 28) & 1) != 0);
        _fma3   = _avx && (((leaf1_ecx >> 12) & 1) != 0);
    }

    if (leaf_count >= 7) {
        std::memset(registers.data(), 0, registers.size() * sizeof(u32));
        cpuidex(registers.data(), 7, 0);

        const auto leaf7_ebx = registers[1];
        const auto leaf7_ecx = registers[2];
        
        _avx2           = _avx && (((leaf7_ebx >> 5) & 1) != 0);

        _avx512f        = avx512_os_support && (((leaf7_ebx >> 16) & 1) != 0);
        _avx512dq       = _avx512f && (((leaf7_ebx >> 17) & 1) != 0);
        _avx512bw       = _avx512f && (((leaf7_ebx >> 30) & 1) != 0);
        _avx512pf       = _avx512f && (((leaf7_ebx >> 26) & 1) != 0);
        _avx512er       = _avx512f && (((leaf7_ebx >> 27) & 1) != 0);
        _avx512cd       = _avx512f && (((leaf7_ebx >> 28) & 1) != 0);
        _avx512vl       = _avx512f && (((leaf7_ebx >> 31) & 1) != 0);
        _erms           = ((leaf7_ebx >> 9) & 1) != 0;

        _avx512vbmi     = _avx512bw && (((leaf7_ecx >> 1) & 1) != 0);
        _avx512vbmi2    = _avx512bw && (((leaf7_ecx >> 6) & 1) != 0);
    }

    _all = (i32(_sse) << static_cast<u32>(features::SSE)) |
        (i32(_sse2) << static_cast<u32>(features::SSE2)) |
        (i32(_sse3) << static_cast<u32>(features::SSE3)) |
        (i32(_ssse3) << static_cast<u32>(features::SSSE3)) |
        (i32(_sse41) << static_cast<u32>(features::SSE41)) |
        (i32(_sse42) << static_cast<u32>(features::SSE42)) |
        (i32(_avx) << static_cast<u32>(features::AVX)) |
        (i32(_avx2) << static_cast<u32>(features::AVX2)) |
        (i32(_fma3) << static_cast<u32>(features::FMA3)) |
        (i32(_avx512f) << static_cast<u32>(features::AVX512F)) |
        (i32(_avx512bw) << static_cast<u32>(features::AVX512BW)) |
        (i32(_avx512pf) << static_cast<u32>(features::AVX512PF)) |
        (i32(_avx512er) << static_cast<u32>(features::AVX512ER)) |
        (i32(_avx512cd) << static_cast<u32>(features::AVX512CD)) |
        (i32(_avx512vl) << static_cast<u32>(features::AVX512VL)) |
        (i32(_avx512dq) << static_cast<u32>(features::AVX512DQ)) |
        (i32(_avx512vbmi) << static_cast<u32>(features::AVX512VBMI)) |
        (i32(_avx512vbmi2) << static_cast<u32>(features::AVX512VBMI2)) |
        (i32(_popcnt) << static_cast<u32>(features::POPCNT)) |
        (i32(_erms) << static_cast<u32>(features::ERMS));
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

bool ProcessorFeatures::FMA3() noexcept {
    return _processorFeaturesInternal._fma3;
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

template <arch::ISA Feature>
bool ProcessorFeatures::isSupported() noexcept {
    if      constexpr (Feature == ISA::None)
        return false;
    else if constexpr (Feature == ISA::SSE)
        return _processorFeaturesInternal._sse;
    else if constexpr (Feature == ISA::SSE2)
        return _processorFeaturesInternal._sse2;
    else if constexpr (Feature == ISA::SSE3)
        return _processorFeaturesInternal._sse3;
    else if constexpr (Feature == ISA::SSSE3)
        return _processorFeaturesInternal._ssse3;
    else if constexpr (Feature == ISA::SSE41)
        return _processorFeaturesInternal._sse41;
    else if constexpr (Feature == ISA::SSE42)
        return _processorFeaturesInternal._sse42;
    else if constexpr (Feature == ISA::AVX)
        return _processorFeaturesInternal._avx;
    else if constexpr (Feature == ISA::AVX2)
        return _processorFeaturesInternal._avx2;
    else if constexpr (Feature == ISA::FMA3)
        return _processorFeaturesInternal._fma3;
    else if constexpr (Feature == ISA::AVX2FMA3)
        return _processorFeaturesInternal._avx2 && _processorFeaturesInternal._fma3;
    else if constexpr (Feature == ISA::AVX512F)
        return _processorFeaturesInternal._avx512f;
    else if constexpr (Feature == ISA::AVX512BW)
        return _processorFeaturesInternal._avx512f && _processorFeaturesInternal._avx512bw;
    else if constexpr (Feature == ISA::AVX512DQ)
        return _processorFeaturesInternal._avx512f && _processorFeaturesInternal._avx512dq;
    else if constexpr (Feature == ISA::AVX512BWDQ)
        return _processorFeaturesInternal._avx512f && _processorFeaturesInternal._avx512dq && _processorFeaturesInternal._avx512bw;
    else if constexpr (Feature == ISA::AVX512VLF)
        return _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512f;
    else if constexpr (Feature == ISA::AVX512VLBW)
        return _processorFeaturesInternal._avx512f && _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512bw;
    else if constexpr (Feature == ISA::AVX512VLDQ)
        return _processorFeaturesInternal._avx512f && _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq;
    else if constexpr (Feature == ISA::AVX512VLBWDQ)
        return _processorFeaturesInternal._avx512f && _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq && _processorFeaturesInternal._avx512bw;
    else if constexpr (Feature == ISA::AVX512VBMI)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi;
    else if constexpr (Feature == ISA::AVX512VBMI2)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vbmi2;
    else if constexpr (Feature == ISA::AVX512VBMIDQ)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512dq;
    else if constexpr (Feature == ISA::AVX512VBMI2DQ)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vbmi2 && _processorFeaturesInternal._avx512dq;
    else if constexpr (Feature == ISA::AVX512VBMIVL)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vl;
    else if constexpr (Feature == ISA::AVX512VBMI2VL)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vbmi2 && _processorFeaturesInternal._avx512vl;
    else if constexpr (Feature == ISA::AVX512VBMIVLDQ)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq;
    else if constexpr (Feature == ISA::AVX512VBMI2VLDQ)
        return _processorFeaturesInternal._avx512bw && _processorFeaturesInternal._avx512vbmi && _processorFeaturesInternal._avx512vbmi2 && _processorFeaturesInternal._avx512vl && _processorFeaturesInternal._avx512dq;
    else
        return false;
}

__RAZE_ARCH_NAMESPACE_END
